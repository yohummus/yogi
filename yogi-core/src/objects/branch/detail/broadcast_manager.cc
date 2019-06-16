/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2019 Johannes Bergmann.
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

#include "broadcast_manager.h"
#include "../../../utils/algorithm.h"

YOGI_DEFINE_INTERNAL_LOGGER("Branch.BroadcastManager")

namespace objects {
namespace branch {
namespace detail {

BroadcastManager::BroadcastManager(ContextPtr context,
                                   ConnectionManager& conn_manager)
    : context_(context), conn_manager_(conn_manager) {}

BroadcastManager::~BroadcastManager() {}

void BroadcastManager::Start(LocalBranchInfoPtr info) {
  SetLoggingPrefix(info->GetLoggingPrefix());
}

api::Result BroadcastManager::SendBroadcast(const network::Payload& payload,
                                            bool block) {
  api::Result result;
  SendBroadcastAsync(payload, block, [&](auto& res, auto) {
    std::lock_guard<std::mutex> lock(this->tx_sync_mutex_);
    result = res;
    this->tx_sync_cv_.notify_all();
  });

  std::unique_lock<std::mutex> lock(tx_sync_mutex_);
  tx_sync_cv_.wait(lock, [&] { return result != api::Result(); });

  return result;
}

BroadcastManager::SendBroadcastOperationId BroadcastManager::SendBroadcastAsync(
    const network::Payload& payload, bool retry, SendBroadcastHandler handler) {
  network::messages::BroadcastOutgoing msg(payload);

  auto oid = conn_manager_.MakeOperationId();

  if (retry) {
    std::shared_ptr<int> pending_handlers;

    std::lock_guard<std::mutex> lock(tx_oids_mutex_);
    conn_manager_.ForeachRunningSession([&](auto& conn) {
      this->SendNowOrLater(&pending_handlers, &msg, conn, handler, oid);
    });

    StoreOidForLaterOrCallHandlerNow(pending_handlers, handler, oid);
  } else {
    bool all_sent = true;
    conn_manager_.ForeachRunningSession([&](auto& conn) {
      if (!conn->TrySend(msg)) {
        all_sent = false;
      }
    });

    if (all_sent) {
      context_->Post([=] { handler(api::kSuccess, oid); });
    } else {
      context_->Post([=] { handler(api::Error(YOGI_ERR_TX_QUEUE_FULL), oid); });
    }
  }

  return oid;
}

bool BroadcastManager::CancelSendBroadcast(SendBroadcastOperationId oid) {
  {
    std::lock_guard<std::mutex> lock(tx_oids_mutex_);
    auto it = utils::find(tx_active_oids_, oid);
    if (it == tx_active_oids_.end()) return false;
    tx_active_oids_.erase(it);
  }

  bool canceled = false;
  conn_manager_.ForeachRunningSession(
      [&](auto& conn) { canceled |= conn->CancelSend(oid); });

  return canceled;
}

void BroadcastManager::ReceiveBroadcast(api::Encoding enc,
                                        boost::asio::mutable_buffer data,
                                        ReceiveBroadcastHandler handler) {
  YOGI_ASSERT(handler);

  std::lock_guard<std::recursive_mutex> lock(rx_mutex_);

  if (rx_handler_) {
    auto old_handler = rx_handler_;
    context_->Post([=] { old_handler(api::Error(YOGI_ERR_CANCELED), {}, 0); });
  }

  rx_enc_ = enc;
  rx_data_ = data;
  rx_handler_ = handler;
}

bool BroadcastManager::CancelReceiveBroadcast() {
  std::lock_guard<std::recursive_mutex> lock(rx_mutex_);

  if (rx_handler_) {
    auto handler = rx_handler_;
    rx_handler_ = {};
    context_->Post([=] { handler(api::Error(YOGI_ERR_CANCELED), {}, 0); });
    return true;
  }

  return false;
}

void BroadcastManager::OnBroadcastReceived(
    const network::messages::BroadcastIncoming& msg,
    const detail::BranchConnectionPtr& conn) {
  std::lock_guard<std::recursive_mutex> lock(rx_mutex_);

  if (rx_handler_) {
    auto handler = rx_handler_;
    rx_handler_ = {};
    std::size_t n = 0;
    auto res = msg.GetPayload().SerializeToUserBuffer(rx_data_, rx_enc_, &n);
    handler(res, conn->GetRemoteBranchInfo()->GetUuid(), n);
  }
}

void BroadcastManager::SendNowOrLater(SharedCounter* pending_handlers,
                                      network::OutgoingMessage* msg,
                                      BranchConnectionPtr conn,
                                      SendBroadcastHandler handler,
                                      SendBroadcastOperationId oid) {
  try {
    if (!conn->TrySend(*msg)) {
      CreateAndIncrementCounter(pending_handlers);

      try {
        auto& pending_handlers_ref = *pending_handlers;
        auto weak_self = std::weak_ptr<BroadcastManager>{shared_from_this()};
        conn->SendAsync(msg, oid, [=](auto&) {
          bool success = false;

          {
            std::lock_guard<std::mutex> lock(tx_oids_mutex_);

            YOGI_ASSERT(pending_handlers_ref);
            bool is_last_handler = --*pending_handlers_ref == 0;
            if (!is_last_handler) return;

            if (auto self = weak_self.lock()) {
              success = self->RemoveActiveOid(oid);
            }
          }

          if (success) {
            handler(api::kSuccess, oid);
          } else {
            handler(api::Error(YOGI_ERR_CANCELED), oid);
          }
        });
      } catch (...) {
        --**pending_handlers;
        throw;
      }
    }
  } catch (const api::Error& err) {
    LOG_ERR("Could not send broadcast to " << conn << ": " << err);
  }
}

void BroadcastManager::StoreOidForLaterOrCallHandlerNow(
    SharedCounter pending_handlers, SendBroadcastHandler handler,
    SendBroadcastOperationId oid) {
  if (pending_handlers) {
    tx_active_oids_.push_back(oid);
  } else {
    context_->Post([=] { handler(api::kSuccess, oid); });
  }
}

void BroadcastManager::CreateAndIncrementCounter(SharedCounter* counter) {
  if (*counter) {
    ++**counter;
  } else {
    *counter = std::make_shared<int>(1);
  }
}

bool BroadcastManager::RemoveActiveOid(SendBroadcastOperationId oid) {
  auto it = utils::find(tx_active_oids_, oid);
  if (it != tx_active_oids_.end()) {
    tx_active_oids_.erase(it);
    return true;
  }

  return false;
}

}  // namespace detail
}  // namespace branch
}  // namespace objects
