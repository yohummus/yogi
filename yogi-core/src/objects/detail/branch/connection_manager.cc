/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "connection_manager.h"
#include "../../../utils/crypto.h"
#include "../../../network/ip.h"
#include "../../../api/constants.h"
#include "../../../utils/json_helpers.h"

#include <boost/uuid/uuid_io.hpp>

YOGI_DEFINE_INTERNAL_LOGGER("Branch.ConnectionManager")

namespace objects {
namespace detail {

ConnectionManager::ConnectionManager(
    ContextPtr context, const nlohmann::json& cfg,
    ConnectionChangedHandler connection_changed_handler,
    MessageReceiveHandler message_handler)
    : context_(context),
      connection_changed_handler_(connection_changed_handler),
      message_handler_(message_handler),
      acceptor_(context->IoContext()),
      last_op_tag_(0),
      observed_events_(api::kNoEvent) {
  adv_ep_ = utils::ExtractUdpEndpoint(cfg, "advertising_address",
                                      api::kDefaultAdvAddress,
                                      "advertising_port", api::kDefaultAdvPort);
  adv_ifs_ = utils::GetFilteredNetworkInterfaces(
      utils::ExtractArrayOfStrings(cfg, "advertising_interfaces",
                                   api::kDefaultAdvInterfaces),
      adv_ep_.protocol());

  password_hash_ = utils::MakeSharedByteVector(
      utils::MakeSha256(cfg.value("network_password", std::string{})));

  adv_sender_ = std::make_shared<AdvertisingSender>(context, adv_ep_);
  adv_receiver_ = std::make_shared<AdvertisingReceiver>(
      context, adv_ep_,
      [&](auto& uuid, auto& ep) { this->OnAdvertisementReceived(uuid, ep); });

  YOGI_ASSERT(adv_ep_.port() != 0);
  using namespace boost::asio::ip;
  SetupAcceptor(adv_ep_.protocol() == udp::v4() ? tcp::v4() : tcp::v6());
}

ConnectionManager::~ConnectionManager() { CancelAwaitEvent(); }

void ConnectionManager::Start(LocalBranchInfoPtr info) {
  info_ = info;
  SetLoggingPrefix(info->GetLoggingPrefix());

  StartAccept();
  adv_sender_->Start(info);
  adv_receiver_->Start(info);

  LOG_DBG("Started ConnectionManager with TCP server port "
          << info_->GetTcpServerPort()
          << (info_->GetGhostMode() ? " in ghost mode" : ""));
}

ConnectionManager::BranchInfoStringsList
ConnectionManager::MakeConnectedBranchesInfoStrings() const {
  BranchInfoStringsList branches;

  std::lock_guard<std::mutex> lock(connections_mutex_);
  for (auto& entry : connections_) {
    YOGI_ASSERT(entry.second);
    if (entry.second->SessionRunning()) {
      branches.push_back(
          std::make_pair(entry.first, entry.second->MakeInfoString()));
    }
  }

  return branches;
}

bool ConnectionManager::AwaitEventAsync(api::BranchEvents events,
                                        BranchEventHandler handler) {
  std::lock_guard<std::recursive_mutex> lock(event_mutex_);

  bool canceled = false;
  if (event_handler_) {
    canceled = true;
    auto old_handler = event_handler_;
    context_->Post([old_handler] {
      old_handler(api::Error(YOGI_ERR_CANCELED), api::kNoEvent, api::kSuccess,
                  {}, {});
    });
  }

  observed_events_ = events;
  event_handler_ = handler;

  return canceled;
}

bool ConnectionManager::CancelAwaitEvent() {
  return AwaitEventAsync(api::kNoEvent, {});
}

ConnectionManager::OperationTag ConnectionManager::MakeOperationId() {
  OperationTag tag;
  do {
    tag = ++last_op_tag_;
  } while (tag == 0);

  return tag;
}

void ConnectionManager::SetupAcceptor(const boost::asio::ip::tcp& protocol) {
  boost::system::error_code ec;
  acceptor_.open(protocol, ec);
  if (ec) throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);

  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
  if (ec) throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);

  bool bound_at_least_once = false;
  unsigned short port = 0;
  for (auto& info : adv_ifs_) {
    for (auto& addr : info.addresses) {
      acceptor_.bind(boost::asio::ip::tcp::endpoint(addr, port), ec);
      if (ec) {
        LOG_ERR("Could not bind to interface "
                << addr
                << " for branch connections. This interface will be ignored.");
        continue;
      }
      LOG_IFO("Using interface " << addr << " for branch connections.");

      bound_at_least_once = true;
    }
  }

  if (bound_at_least_once) {
    acceptor_.listen(acceptor_.max_listen_connections, ec);
    if (ec) throw api::Error(YOGI_ERR_LISTEN_SOCKET_FAILED);
  } else {
    LOG_ERR("No network interfaces available for branch connections.");
  }
}

void ConnectionManager::StartAccept() {
  auto weak_self = MakeWeakPtr();
  accept_guard_ = network::TcpTransport::AcceptAsync(
      context_, &acceptor_, info_->GetTimeout(),
      info_->GetTransceiveByteLimit(), [=](auto& res, auto transport, auto) {
        auto self = weak_self.lock();
        if (!self) return;

        self->OnAcceptFinished(res, transport);
      });
}

void ConnectionManager::OnAcceptFinished(const api::Result& res,
                                         network::TcpTransportPtr transport) {
  if (res.IsError()) {
    LOG_ERR("Accepting incoming TCP connection failed: "
            << res << ". No more connections will be accepted.");
    return;
  }

  LOG_DBG("Accepted incoming TCP connection from "
          << network::MakeIpAddressString(transport->GetPeerEndpoint()));

  StartExchangeBranchInfo(transport, transport->GetPeerEndpoint().address(),
                          {});
  StartAccept();
}

void ConnectionManager::OnAdvertisementReceived(
    const boost::uuids::uuid& adv_uuid,
    const boost::asio::ip::tcp::endpoint& ep) {
  std::lock_guard<std::mutex> lock(connections_mutex_);
  if (connections_.count(adv_uuid)) return;
  if (blacklisted_uuids_.count(adv_uuid)) return;
  if (pending_connects_.count(adv_uuid)) return;

  LOG_DBG("Attempting to connect to ["
          << adv_uuid << "] on " << network::MakeIpAddressString(ep) << ":"
          << ep.port());

  auto weak_self = MakeWeakPtr();
  auto guard = network::TcpTransport::ConnectAsync(
      context_, ep, info_->GetTimeout(), info_->GetTransceiveByteLimit(),
      [=](auto& res, auto transport, auto guard) {
        auto self = weak_self.lock();
        if (!self) return;

        self->connect_guards_.erase(guard);
        self->OnConnectFinished(res, adv_uuid, transport);
      });

  pending_connects_.insert(adv_uuid);
  connect_guards_.insert(guard);

  EmitBranchEvent(api::kBranchDiscoveredEvent, api::kSuccess, adv_uuid, [&] {
    return nlohmann::json{
        {"uuid", boost::uuids::to_string(adv_uuid)},
        {"tcp_server_address", network::MakeIpAddressString(ep)},
        {"tcp_server_port", ep.port()}};
  });
}

void ConnectionManager::OnConnectFinished(const api::Result& res,
                                          const boost::uuids::uuid& adv_uuid,
                                          network::TcpTransportPtr transport) {
  if (res.IsError()) {
    EmitBranchEvent(api::kBranchQueriedEvent, res, adv_uuid);
    pending_connects_.erase(adv_uuid);
    return;
  }

  LOG_DBG("TCP connection to " << *transport << " established successfully");

  StartExchangeBranchInfo(transport, transport->GetPeerEndpoint().address(),
                          adv_uuid);
}

void ConnectionManager::StartExchangeBranchInfo(
    network::TransportPtr transport,
    const boost::asio::ip::address& peer_address,
    const boost::uuids::uuid& adv_uuid) {
  auto conn = MakeConnectionAndKeepItAlive(peer_address, transport);
  auto weak_conn = BranchConnectionWeakPtr(conn);
  conn->ExchangeBranchInfo([this, weak_conn, adv_uuid](auto& res) {
    YOGI_ASSERT(weak_conn.lock());
    this->OnExchangeBranchInfoFinished(res, weak_conn.lock(), adv_uuid);
    this->StopKeepingConnectionAlive(weak_conn);
    this->pending_connects_.erase(adv_uuid);
  });
}

void ConnectionManager::OnExchangeBranchInfoFinished(
    const api::Result& res, BranchConnectionPtr conn,
    const boost::uuids::uuid& adv_uuid) {
  if (!CheckExchangeBranchInfoError(res, conn)) {
    return;
  }

  auto remote_info = conn->GetRemoteBranchInfo();
  auto& remote_uuid = remote_info->GetUuid();

  if (!conn->CreatedFromIncomingConnectionRequest() &&
      !VerifyUuidsMatch(remote_uuid, adv_uuid)) {
    return;
  }

  if (!VerifyUuidNotBlacklisted(remote_uuid)) {
    return;
  }

  LOG_DBG(
      "Successfully exchanged branch info with "
      << remote_info << " (source: "
      << (conn->CreatedFromIncomingConnectionRequest() ? "server" : "client")
      << ")");

  std::lock_guard<std::mutex> lock(connections_mutex_);
  auto con_res = connections_.insert(std::make_pair(remote_uuid, conn));
  bool conn_already_exists = !con_res.second;

  if (!VerifyConnectionHasHigherPriority(conn_already_exists, conn)) {
    return;
  }

  con_res.first->second = conn;

  if (!conn_already_exists) {
    EmitBranchEvent(api::kBranchQueriedEvent, api::kSuccess, remote_uuid,
                    [&] { return remote_info->ToJson(); });

    // If a connection already exists, the following check has already been
    // performed successfully, so we don't do it again
    auto chk_res = CheckRemoteBranchInfo(remote_info);
    if (chk_res.IsError()) {
      EmitBranchEvent(api::kConnectFinishedEvent, chk_res, remote_uuid);
      return;
    }
  }

  if (info_->GetGhostMode()) {
    blacklisted_uuids_.insert(remote_uuid);
  } else {
    StartAuthenticate(conn);
  }
}

bool ConnectionManager::CheckExchangeBranchInfoError(
    const api::Result& res, const BranchConnectionPtr& conn) {
  if (res.IsSuccess()) {
    return true;
  }

  if (conn->CreatedFromIncomingConnectionRequest()) {
    LOG_ERR("Exchanging branch info with server connection from "
            << conn->GetPeerDescription() << " failed: " << res);
  } else {
    LOG_ERR("Exchanging branch info with client connection to "
            << conn->GetPeerDescription() << " failed: " << res);
  }

  return false;
}

bool ConnectionManager::VerifyUuidsMatch(const boost::uuids::uuid& remote_uuid,
                                         const boost::uuids::uuid& adv_uuid) {
  if (remote_uuid == adv_uuid) {
    return true;
  }

  LOG_WRN("Dropping connection since branch info UUID ["
          << remote_uuid << "] does not match advertised UUID [" << adv_uuid
          << "]. This will likely be fixed with the next  connection attempt.");
  return false;
}

bool ConnectionManager::VerifyUuidNotBlacklisted(
    const boost::uuids::uuid& uuid) {
  if (!blacklisted_uuids_.count(uuid)) {
    return true;
  }

  LOG_DBG("Dropping connection to [" << uuid << "] since it is blacklisted");

  return false;
}

bool ConnectionManager::VerifyConnectionHasHigherPriority(
    bool conn_already_exists, const BranchConnectionPtr& conn) {
  if (!conn_already_exists) {
    return true;
  }

  auto& remote_uuid = conn->GetRemoteBranchInfo()->GetUuid();
  YOGI_ASSERT(connections_[remote_uuid]);

  if ((remote_uuid < info_->GetUuid()) ==
      conn->CreatedFromIncomingConnectionRequest()) {
    return true;
  }

  LOG_DBG(
      "Dropping TCP "
      << (conn->CreatedFromIncomingConnectionRequest() ? "server" : "client")
      << " connection to " << conn
      << " since a connection with a higher priority already exists")

  return false;
}

api::Result ConnectionManager::CheckRemoteBranchInfo(
    const BranchInfoPtr& remote_info) {
  if (remote_info->GetNetworkName() != info_->GetNetworkName()) {
    blacklisted_uuids_.insert(remote_info->GetUuid());
    return api::Error(YOGI_ERR_NET_NAME_MISMATCH);
  }

  if (remote_info->GetName() == info_->GetName()) {
    blacklisted_uuids_.insert(remote_info->GetUuid());
    return api::Error(YOGI_ERR_DUPLICATE_BRANCH_NAME);
  }

  if (remote_info->GetPath() == info_->GetPath()) {
    blacklisted_uuids_.insert(remote_info->GetUuid());
    return api::Error(YOGI_ERR_DUPLICATE_BRANCH_PATH);
  }

  for (auto& entry : connections_) {
    auto branch_info = entry.second->GetRemoteBranchInfo();
    if (branch_info == remote_info) continue;

    YOGI_ASSERT(remote_info->GetUuid() != branch_info->GetUuid());

    if (remote_info->GetName() == branch_info->GetName()) {
      return api::Error(YOGI_ERR_DUPLICATE_BRANCH_NAME);
    }

    if (remote_info->GetPath() == branch_info->GetPath()) {
      return api::Error(YOGI_ERR_DUPLICATE_BRANCH_PATH);
    }
  }

  return api::kSuccess;
}

void ConnectionManager::StartAuthenticate(BranchConnectionPtr conn) {
  auto weak_conn = BranchConnectionWeakPtr(conn);
  conn->Authenticate(password_hash_, [this, weak_conn](auto& res) {
    YOGI_ASSERT(weak_conn.lock());
    this->OnAuthenticateFinished(res, weak_conn.lock());
  });
}

void ConnectionManager::OnAuthenticateFinished(const api::Result& res,
                                               BranchConnectionPtr conn) {
  auto& uuid = conn->GetRemoteBranchInfo()->GetUuid();

  if (res.IsError()) {
    if (res == api::Error(YOGI_ERR_PASSWORD_MISMATCH)) {
      blacklisted_uuids_.insert(uuid);
    }

    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.erase(uuid);

    EmitBranchEvent(api::kConnectFinishedEvent, res, uuid);
  } else {
    LOG_DBG("Successfully authenticated with " << conn->GetRemoteBranchInfo());
    StartSession(conn);
  }
}

void ConnectionManager::StartSession(BranchConnectionPtr conn) {
  auto weak_conn = BranchConnectionWeakPtr(conn);
  conn->RunSession(
      [this, weak_conn](auto& msg) {
        YOGI_ASSERT(weak_conn.lock());
        this->message_handler_(msg, weak_conn.lock());
      },
      [this, weak_conn](auto& res) {
        YOGI_ASSERT(weak_conn.lock());
        this->OnSessionTerminated(res.ToError(), weak_conn.lock());
      });

  EmitBranchEvent(api::kConnectFinishedEvent, api::kSuccess,
                  conn->GetRemoteBranchInfo()->GetUuid());

  LOG_DBG("Successfully started session for " << conn->GetRemoteBranchInfo());

  connection_changed_handler_(api::kSuccess, conn);
}

void ConnectionManager::OnSessionTerminated(const api::Error& err,
                                            BranchConnectionPtr conn) {
  LOG_DBG("Session for " << conn->GetRemoteBranchInfo()
                         << " terminated: " << err);

  EmitBranchEvent(api::kConnectionLostEvent, err,
                  conn->GetRemoteBranchInfo()->GetUuid());

  std::lock_guard<std::mutex> lock(connections_mutex_);
  connections_.erase(conn->GetRemoteBranchInfo()->GetUuid());

  connection_changed_handler_(err, conn);
}

BranchConnectionPtr ConnectionManager::MakeConnectionAndKeepItAlive(
    const boost::asio::ip::address& peer_address,
    network::TransportPtr transport) {
  auto conn =
      std::make_shared<BranchConnection>(transport, peer_address, info_);
  connections_kept_alive_.insert(conn);
  return conn;
}

BranchConnectionPtr ConnectionManager::StopKeepingConnectionAlive(
    const BranchConnectionWeakPtr& weak_conn) {
  auto conn = weak_conn.lock();
  YOGI_ASSERT(conn);
  connections_kept_alive_.erase(conn);
  return conn;
}

template <typename Fn>
void ConnectionManager::EmitBranchEvent(api::BranchEvents event,
                                        const api::Result& ev_res,
                                        const boost::uuids::uuid& uuid,
                                        Fn make_json_fn) {
  LogBranchEvent(event, ev_res, make_json_fn);

  std::lock_guard<std::recursive_mutex> lock(event_mutex_);

  if (!event_handler_) return;
  if (!(observed_events_ & event)) return;

  auto handler = event_handler_;
  event_handler_ = {};
  handler(api::kSuccess, event, ev_res, uuid, make_json_fn().dump());
}

void ConnectionManager::EmitBranchEvent(api::BranchEvents event,
                                        const api::Result& ev_res,
                                        const boost::uuids::uuid& uuid) {
  return EmitBranchEvent(event, ev_res, uuid, [&] {
    return nlohmann::json{{"uuid", boost::uuids::to_string(uuid)}};
  });
}

template <typename Fn>
void ConnectionManager::LogBranchEvent(api::BranchEvents event,
                                       const api::Result& ev_res,
                                       Fn make_json_fn) {
  switch (event) {
    case api::kNoEvent:
    case api::kAllEvents:
      break;

    case api::kBranchDiscoveredEvent:
      LOG_DBG("Event: YOGI_BEV_BRANCH_DISCOVERED; ev_res=\""
              << ev_res << "; json=\"" << make_json_fn() << "\"");
      break;

    case api::kBranchQueriedEvent:
      LOG_IFO("Event: YOGI_BEV_BRANCH_QUERIED; ev_res=\""
              << ev_res << "; json=\"" << make_json_fn() << "\"");
      break;

    case api::kConnectFinishedEvent:
      LOG_IFO("Event: YOGI_BEV_CONNECT_FINISHED; ev_res=\""
              << ev_res << "; json=\"" << make_json_fn() << "\"");
      break;

    case api::kConnectionLostEvent:
      LOG_WRN("Event: YOGI_BEV_CONNECTION_LOST; ev_res=\""
              << ev_res << "; json=\"" << make_json_fn() << "\"");
      break;
  }
}

}  // namespace detail
}  // namespace objects
