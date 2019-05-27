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

#include "branch.h"
#include "../network/ip.h"
#include "../api/constants.h"
#include "../utils/system.h"

#include <chrono>
using namespace std::chrono_literals;
using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("Branch")

namespace objects {

Branch::Branch(ContextPtr context, const nlohmann::json& cfg)
    : context_(context),
      connection_manager_(std::make_shared<detail::ConnectionManager>(
          context, cfg,
          [&](auto& res, auto conn) { this->OnConnectionChanged(res, conn); },
          [&](auto& msg, auto& conn) { this->OnMessageReceived(msg, conn); })),
      info_(std::make_shared<detail::LocalBranchInfo>(
          cfg, connection_manager_->GetAdvertisingInterfaces(),
          connection_manager_->GetTcpServerEndpoint())),
      broadcast_manager_(std::make_shared<detail::BroadcastManager>(
          context, *connection_manager_)) {
  if (info_->GetName().empty() || info_->GetNetworkName().empty() ||
      info_->GetPath().empty() || info_->GetPath().front() != '/' ||
      info_->GetAdvertisingInterval() < 1ms || info_->GetTimeout() < 1ms) {
    throw api::Error(YOGI_ERR_INVALID_PARAM);
  }
}

void Branch::Start() {
  SetLoggingPrefix(info_->GetLoggingPrefix());
  connection_manager_->Start(info_);
  broadcast_manager_->Start(info_);
}

ContextPtr Branch::GetContext() const { return context_; }

const boost::uuids::uuid& Branch::GetUuid() const { return info_->GetUuid(); }

std::string Branch::MakeInfoString() const { return info_->ToJson().dump(); }

Branch::BranchInfoStringsList Branch::MakeConnectedBranchesInfoStrings() const {
  return connection_manager_->MakeConnectedBranchesInfoStrings();
}

void Branch::AwaitEventAsync(api::BranchEvents events,
                             BranchEventHandler handler) {
  connection_manager_->AwaitEventAsync(events, handler);
}

bool Branch::CancelAwaitEvent() {
  return connection_manager_->CancelAwaitEvent();
}

Branch::SendBroadcastOperationId Branch::SendBroadcastAsync(
    const network::Payload& payload, bool retry, SendBroadcastHandler handler) {
  return broadcast_manager_->SendBroadcastAsync(payload, retry, handler);
}

api::Result Branch::SendBroadcast(const network::Payload& payload, bool block) {
  return broadcast_manager_->SendBroadcast(payload, block);
}

bool Branch::CancelSendBroadcast(SendBroadcastOperationId oid) {
  return broadcast_manager_->CancelSendBroadcast(oid);
}

void Branch::ReceiveBroadcast(api::Encoding enc,
                              boost::asio::mutable_buffer data,
                              ReceiveBroadcastHandler handler) {
  broadcast_manager_->ReceiveBroadcast(enc, data, handler);
}

bool Branch::CancelReceiveBroadcast() {
  return broadcast_manager_->CancelReceiveBroadcast();
}

void Branch::OnConnectionChanged(const api::Result& res,
                                 const detail::BranchConnectionPtr& conn) {
  LOG_IFO("Connection to " << conn->GetRemoteBranchInfo()
                           << " changed: " << res);
  // TODO
}

void Branch::OnMessageReceived(const network::IncomingMessage& msg,
                               const detail::BranchConnectionPtr& conn) {
  using namespace network;

  LOG_TRC("Message received: " << msg);

  switch (msg.GetType()) {
    case MessageType::kHeartbeat:
      break;

    case MessageType::kBroadcast:
      broadcast_manager_->OnBroadcastReceived(
          static_cast<const messages::BroadcastIncoming&>(msg), conn);
      break;

    default:
      LOG_ERR("Message of unexpected type received: " << msg);
      YOGI_NEVER_REACHED;
      break;
  }
}

}  // namespace objects
