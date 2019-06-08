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
#include "../utils/schema.h"
#include "../utils/system.h"

#include <chrono>
using namespace std::chrono_literals;
using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("Branch")

namespace objects {

Branch::Branch(ContextPtr context, const nlohmann::json& cfg)
    : context_(context) {
  utils::ValidateJson(cfg, "branch_create.schema.json");

  con_man_ = std::make_shared<detail::ConnectionManager>(
      context, cfg,
      [&](auto& res, auto conn) { this->OnConnectionChanged(res, conn); },
      [&](auto& msg, auto& conn) { this->OnMessageReceived(msg, conn); });

  info_ = std::make_shared<detail::LocalBranchInfo>(
      cfg, con_man_->GetAdvertisingInterfaces(),
      con_man_->GetTcpServerEndpoint());

  bc_man_ = std::make_shared<detail::BroadcastManager>(context, *con_man_);
}

void Branch::Start() {
  SetLoggingPrefix(info_->GetLoggingPrefix());
  con_man_->Start(info_);
  bc_man_->Start(info_);
}

ContextPtr Branch::GetContext() const { return context_; }

const boost::uuids::uuid& Branch::GetUuid() const { return info_->GetUuid(); }

std::string Branch::MakeInfoString() const { return info_->ToJson().dump(); }

Branch::BranchInfoStringsList Branch::MakeConnectedBranchesInfoStrings() const {
  return con_man_->MakeConnectedBranchesInfoStrings();
}

void Branch::AwaitEventAsync(api::BranchEvents events,
                             BranchEventHandler handler) {
  con_man_->AwaitEventAsync(events, handler);
}

bool Branch::CancelAwaitEvent() { return con_man_->CancelAwaitEvent(); }

Branch::SendBroadcastOperationId Branch::SendBroadcastAsync(
    const network::Payload& payload, bool retry, SendBroadcastHandler handler) {
  return bc_man_->SendBroadcastAsync(payload, retry, handler);
}

api::Result Branch::SendBroadcast(const network::Payload& payload, bool block) {
  return bc_man_->SendBroadcast(payload, block);
}

bool Branch::CancelSendBroadcast(SendBroadcastOperationId oid) {
  return bc_man_->CancelSendBroadcast(oid);
}

void Branch::ReceiveBroadcast(api::Encoding enc,
                              boost::asio::mutable_buffer data,
                              ReceiveBroadcastHandler handler) {
  bc_man_->ReceiveBroadcast(enc, data, handler);
}

bool Branch::CancelReceiveBroadcast() {
  return bc_man_->CancelReceiveBroadcast();
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
      bc_man_->OnBroadcastReceived(
          static_cast<const messages::BroadcastIncoming&>(msg), conn);
      break;

    default:
      LOG_ERR("Message of unexpected type received: " << msg);
      YOGI_NEVER_REACHED;
      break;
  }
}

}  // namespace objects
