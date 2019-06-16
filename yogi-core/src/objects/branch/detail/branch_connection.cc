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

#include "branch_connection.h"
#include "../../../api/constants.h"
#include "../../../utils/crypto.h"
#include "../../../network/serialize.h"

namespace objects {
namespace branch {
namespace detail {

BranchConnection::BranchConnection(network::TransportPtr transport,
                                   const boost::asio::ip::address& peer_address,
                                   LocalBranchInfoPtr local_info)
    : transport_(transport),
      context_(transport->GetContext()),
      local_info_(local_info),
      peer_address_(peer_address),
      connected_since_(utils::Timestamp::Now()),
      session_running_(false),
      heartbeat_timer_(context_->IoContext()),
      next_result_(api::kSuccess) {}

std::string BranchConnection::MakeInfoString() const {
  auto json = remote_info_->ToJson();
  json["connected_since_"] = connected_since_.ToJavaScriptString();
  return json.dump();
}

void BranchConnection::ExchangeBranchInfo(CompletionHandler handler) {
  YOGI_ASSERT(!remote_info_);

  auto weak_self = MakeWeakPtr();
  transport_->SendAllAsync(local_info_->MakeInfoMessage(), [=](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      handler(res);
    } else {
      self->OnInfoSent(handler);
    }
  });
}

void BranchConnection::Authenticate(utils::SharedByteVector password_hash,
                                    CompletionHandler handler) {
  YOGI_ASSERT(remote_info_);

  if (!CheckNextResult(handler)) return;

  auto my_challenge =
      utils::MakeSharedByteVector(utils::GenerateRandomBytes(8));

  auto weak_self = MakeWeakPtr();
  transport_->SendAllAsync(my_challenge, [=](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      handler(res);
    } else {
      self->OnChallengeSent(my_challenge, password_hash, handler);
    }
  });
}

void BranchConnection::RunSession(MessageReceiveHandler rcv_handler,
                                  CompletionHandler session_handler) {
  YOGI_ASSERT(remote_info_);
  YOGI_ASSERT(!SessionRunning());

  if (!CheckNextResult(session_handler)) return;

  msg_transport_ = std::make_shared<network::MessageTransport>(
      transport_, local_info_->GetTxQueueSize(), local_info_->GetRxQueueSize());
  msg_transport_->Start();

  RestartHeartbeatTimer();
  StartReceive(utils::MakeSharedByteVector());
  session_running_ = true;
  session_handler_ = session_handler;
  rcv_handler_ = rcv_handler;
}

void BranchConnection::OnInfoSent(CompletionHandler handler) {
  auto weak_self = MakeWeakPtr();
  auto buffer = utils::MakeSharedByteVector(BranchInfo::kInfoMessageHeaderSize);
  transport_->ReceiveAllAsync(buffer, [=](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      handler(res);
    } else {
      self->OnInfoHeaderReceived(buffer, handler);
    }
  });
}

void BranchConnection::OnInfoHeaderReceived(utils::SharedByteVector buffer,
                                            CompletionHandler handler) {
  std::size_t body_size;
  auto res =
      RemoteBranchInfo::DeserializeInfoMessageBodySize(&body_size, *buffer);
  if (res.IsError()) {
    handler(res);
    return;
  }

  if (body_size > api::kMaxMessagePayloadSize) {
    handler(api::Error(YOGI_ERR_PAYLOAD_TOO_LARGE));
    return;
  }

  auto weak_self = MakeWeakPtr();
  buffer->resize(BranchInfo::kInfoMessageHeaderSize + body_size);
  transport_->ReceiveAllAsync(
      boost::asio::buffer(*buffer) + BranchInfo::kInfoMessageHeaderSize,
      [=](auto& res) {
        auto self = weak_self.lock();
        if (!self) return;

        if (res.IsError()) {
          handler(res);
        } else {
          self->OnInfoBodyReceived(buffer, handler);
        }
      });
}

void BranchConnection::OnInfoBodyReceived(utils::SharedByteVector info_msg,
                                          CompletionHandler handler) {
  try {
    remote_info_ = std::make_shared<RemoteBranchInfo>(*info_msg, peer_address_);

    if (remote_info_->GetUuid() == local_info_->GetUuid()) {
      throw api::Error(YOGI_ERR_LOOPBACK_CONNECTION);
    }
  } catch (const api::Error& err) {
    handler(err);
    return;
  }

  auto weak_self = MakeWeakPtr();
  transport_->SendAllAsync(ack_msg_.SerializeShared(), [=](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      handler(res);
    } else {
      self->OnInfoAckSent(handler);
    }
  });
}

void BranchConnection::OnInfoAckSent(CompletionHandler handler) {
  auto weak_self = MakeWeakPtr();
  auto ack_msg = utils::MakeSharedByteVector(ack_msg_.GetSize());
  transport_->ReceiveAllAsync(ack_msg, [=](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    self->OnInfoAckReceived(res, ack_msg, handler);
  });
}

void BranchConnection::OnInfoAckReceived(const api::Result& res,
                                         utils::SharedByteVector ack_msg,
                                         CompletionHandler handler) {
  CheckAckAndSetNextResult(res, *ack_msg);
  handler(api::kSuccess);
}

void BranchConnection::OnChallengeSent(utils::SharedByteVector my_challenge,
                                       utils::SharedByteVector password_hash,
                                       CompletionHandler handler) {
  auto weak_self = MakeWeakPtr();
  auto remote_challenge = utils::MakeSharedByteVector(my_challenge->size());
  transport_->ReceiveAllAsync(remote_challenge, [=](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      handler(res);
    } else {
      this->OnChallengeReceived(remote_challenge, my_challenge, password_hash,
                                handler);
    }
  });
}

void BranchConnection::OnChallengeReceived(
    utils::SharedByteVector remote_challenge,
    utils::SharedByteVector my_challenge, utils::SharedByteVector password_hash,
    CompletionHandler handler) {
  auto weak_self = MakeWeakPtr();
  auto my_solution = SolveChallenge(*my_challenge, *password_hash);
  auto remote_solution = SolveChallenge(*remote_challenge, *password_hash);
  transport_->SendAllAsync(remote_solution, [=](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      handler(res);
    } else {
      self->OnSolutionSent(my_solution, handler);
    }
  });
}

utils::SharedByteVector BranchConnection::SolveChallenge(
    const utils::ByteVector& challenge,
    const utils::ByteVector& password_hash) const {
  auto data = challenge;
  data.insert(data.end(), password_hash.begin(), password_hash.end());
  return utils::MakeSharedByteVector(utils::MakeSha256(data));
}

void BranchConnection::OnSolutionSent(utils::SharedByteVector my_solution,
                                      CompletionHandler handler) {
  auto weak_self = MakeWeakPtr();
  auto received_solution = utils::MakeSharedByteVector(my_solution->size());
  transport_->ReceiveAllAsync(received_solution, [=](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      handler(res);
    } else {
      self->OnSolutionReceived(received_solution, my_solution, handler);
    }
  });
}

void BranchConnection::OnSolutionReceived(
    utils::SharedByteVector received_solution,
    utils::SharedByteVector my_solution, CompletionHandler handler) {
  auto weak_self = MakeWeakPtr();
  bool solutions_match = *received_solution == *my_solution;
  transport_->SendAllAsync(ack_msg_.SerializeShared(), [=](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      handler(res);
    } else {
      self->OnSolutionAckSent(solutions_match, handler);
    }
  });
}

void BranchConnection::OnSolutionAckSent(bool solutions_match,
                                         CompletionHandler handler) {
  auto weak_self = MakeWeakPtr();
  auto ack_msg = utils::MakeSharedByteVector(ack_msg_.GetSize());
  transport_->ReceiveAllAsync(ack_msg, [=](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    self->OnSolutionAckReceived(res, solutions_match, ack_msg, handler);
  });
}

void BranchConnection::OnSolutionAckReceived(const api::Result& res,
                                             bool solutions_match,
                                             utils::SharedByteVector ack_msg,
                                             CompletionHandler handler) {
  CheckAckAndSetNextResult(res, *ack_msg);

  if (!solutions_match) {
    handler(api::Error(YOGI_ERR_PASSWORD_MISMATCH));
  } else {
    handler(api::kSuccess);
  }
}

void BranchConnection::RestartHeartbeatTimer() {
  YOGI_ASSERT((remote_info_->GetTimeout() / 2).count() > 0);
  heartbeat_timer_.expires_from_now(remote_info_->GetTimeout() / 2);

  auto weak_self = MakeWeakPtr();
  heartbeat_timer_.async_wait([weak_self](auto& ec) {
    if (ec == boost::asio::error::operation_aborted) return;

    auto self = weak_self.lock();
    if (!self) return;

    self->OnHeartbeatTimerExpired();
  });
}

void BranchConnection::OnHeartbeatTimerExpired() {
  TrySend(heartbeat_msg_);
  RestartHeartbeatTimer();
}

void BranchConnection::StartReceive(utils::SharedByteVector buffer) {
  auto weak_self = std::weak_ptr<BranchConnection>(shared_from_this());
  buffer->resize(api::kMinRxQueueSize);  // This will fit an entire message
  msg_transport_->ReceiveAsync(boost::asio::buffer(*buffer),
                               [=](auto& res, auto msg_size) {
                                 auto self = weak_self.lock();
                                 if (!self) return;

                                 if (res.IsError()) {
                                   self->OnSessionError(res.ToError());
                                 } else {
                                   buffer->resize(msg_size);
                                   self->OnMessageReceived(buffer);
                                   self->StartReceive(buffer);
                                 }
                               });
}

void BranchConnection::OnSessionError(const api::Error& err) {
  heartbeat_timer_.cancel();
  session_handler_(err);
}

void BranchConnection::CheckAckAndSetNextResult(
    const api::Result& res, const utils::ByteVector& ack_msg) {
  if (res.IsError()) {
    next_result_ = res;
  } else if (ack_msg.size() != ack_msg_.GetSize() ||
             ack_msg[0] != ack_msg_.Serialize()[0]) {
    next_result_ = api::Error(YOGI_ERR_DESERIALIZE_MSG_FAILED);
  }
}

bool BranchConnection::CheckNextResult(CompletionHandler handler) {
  if (next_result_.IsError()) {
    auto res = next_result_;
    context_->Post([=] { handler(res); });

    return false;
  }

  return true;
}

void BranchConnection::OnMessageReceived(const utils::SharedByteVector& msg) {
  network::IncomingMessage::Deserialize(*msg, rcv_handler_);
}

}  // namespace detail
}  // namespace branch
}  // namespace objects

std::ostream& operator<<(
    std::ostream& os, const objects::branch::detail::BranchConnection& conn) {
  os << conn.GetRemoteBranchInfo();
  return os;
}
