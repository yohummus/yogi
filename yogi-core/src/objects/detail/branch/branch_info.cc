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

#include "branch_info.h"
#include "../../../api/constants.h"
#include "../../../network/ip.h"
#include "../../../network/serialize.h"
#include "../../../utils/system.h"

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace objects {
namespace detail {
namespace {

template <typename Field>
void DeserializeField(Field* field, const utils::ByteVector& msg,
                      utils::ByteVector::const_iterator* it) {
  if (!network::Deserialize<Field>(field, msg, it)) {
    throw api::Error(YOGI_ERR_DESERIALIZE_MSG_FAILED);
  }
}

}  // anonymous namespace

void BranchInfo::PopulateJson() {
  float timeout = -1;
  if (timeout_ != timeout_.max()) {
    timeout = static_cast<float>(timeout_.count()) / 1e9f;
  }

  float adv_interval = -1;
  if (adv_interval_ != adv_interval_.max()) {
    adv_interval = static_cast<float>(adv_interval_.count()) / 1e9f;
  }

  json_ = {
      {"uuid", boost::uuids::to_string(uuid_)},
      {"name", name_},
      {"description", description_},
      {"network_name", net_name_},
      {"path", path_},
      {"hostname", hostname_},
      {"pid", pid_},
      {"tcp_server_address", network::MakeIpAddressString(tcp_ep_)},
      {"tcp_server_port", tcp_ep_.port()},
      {"start_time", start_time_.ToJavaScriptString()},
      {"timeout", timeout},
      {"advertising_interval", adv_interval},
      {"ghost_mode", ghost_mode_},
  };
}

LocalBranchInfo::LocalBranchInfo(
    std::string name, std::string description, std::string net_name,
    std::string path, const std::vector<utils::NetworkInterfaceInfo>& adv_ifs,
    const boost::asio::ip::udp::endpoint& adv_ep,
    const boost::asio::ip::tcp::endpoint& tcp_ep,
    const std::chrono::nanoseconds& timeout,
    const std::chrono::nanoseconds& adv_interval, bool ghost_mode,
    std::size_t tx_queue_size, std::size_t rx_queue_size,
    std::size_t transceive_byte_limit) {
  uuid_ = boost::uuids::random_generator()();
  name_ = name;
  description_ = description;
  net_name_ = net_name;
  path_ = path;
  hostname_ = utils::GetHostname();
  pid_ = utils::GetProcessId();
  adv_ifs_ = adv_ifs;
  tcp_ep_ = tcp_ep;
  start_time_ = utils::Timestamp::Now();
  timeout_ = timeout;
  adv_interval_ = adv_interval;
  ghost_mode_ = ghost_mode;
  adv_ep_ = adv_ep;
  tx_queue_size_ = tx_queue_size;
  rx_queue_size_ = rx_queue_size;
  transceive_byte_limit_ = transceive_byte_limit;

  PopulateMessages();
  PopulateJson();
  PopulateJsonWithLocalInfo();
}

void LocalBranchInfo::PopulateMessages() {
  utils::ByteVector buffer{'Y', 'O', 'G', 'I', 0};
  buffer.push_back(api::kVersionMajor);
  buffer.push_back(api::kVersionMinor);
  network::Serialize(&buffer, uuid_);
  network::Serialize(&buffer, tcp_ep_.port());

  YOGI_ASSERT(buffer.size() == kAdvertisingMessageSize);
  adv_msg_ = utils::MakeSharedByteVector(buffer);

  info_msg_ = utils::MakeSharedByteVector(buffer);

  buffer.clear();
  network::Serialize(&buffer, name_);
  network::Serialize(&buffer, description_);
  network::Serialize(&buffer, net_name_);
  network::Serialize(&buffer, path_);
  network::Serialize(&buffer, hostname_);
  network::Serialize(&buffer, pid_);
  network::Serialize(&buffer, start_time_);
  network::Serialize(&buffer, timeout_);
  network::Serialize(&buffer, adv_interval_);
  network::Serialize(&buffer, ghost_mode_);

  network::Serialize(&*info_msg_, buffer.size());
  YOGI_ASSERT(info_msg_->size() == kInfoMessageHeaderSize);
  info_msg_->insert(info_msg_->end(), buffer.begin(), buffer.end());
}

void LocalBranchInfo::PopulateJsonWithLocalInfo() {
  nlohmann::json::array_t ifs;
  for (auto& info : adv_ifs_) {
    nlohmann::json::array_t addrs;
    for (auto& addr : info.addresses) {
      addrs.push_back(addr.to_string());
    }

    auto entry = nlohmann::json{{"name", info.name},
                                {"identifier", info.identifier},
                                {"mac", info.mac},
                                {"addresses", addrs},
                                {"is_loopback", info.is_loopback}};
    ifs.push_back(entry);
  }

  json_["advertising_interfaces"] = ifs;
  json_["advertising_address"] = network::MakeIpAddressString(adv_ep_);
  json_["advertising_port"] = adv_ep_.port();
  json_["tx_queue_size"] = tx_queue_size_;
  json_["rx_queue_size"] = rx_queue_size_;
}

RemoteBranchInfo::RemoteBranchInfo(const utils::ByteVector& info_msg,
                                   const boost::asio::ip::address& addr) {
  unsigned short port;
  auto res = DeserializeAdvertisingMessage(&uuid_, &port, info_msg);
  if (res.IsError()) {
    throw res.ToError();
  }

  tcp_ep_.port(port);
  tcp_ep_.address(addr);

  auto it = info_msg.cbegin() + kInfoMessageHeaderSize;
  DeserializeField(&name_, info_msg, &it);
  DeserializeField(&description_, info_msg, &it);
  DeserializeField(&net_name_, info_msg, &it);
  DeserializeField(&path_, info_msg, &it);
  DeserializeField(&hostname_, info_msg, &it);
  DeserializeField(&pid_, info_msg, &it);
  DeserializeField(&start_time_, info_msg, &it);
  DeserializeField(&timeout_, info_msg, &it);
  DeserializeField(&adv_interval_, info_msg, &it);
  DeserializeField(&ghost_mode_, info_msg, &it);

  PopulateJson();
}

api::Result RemoteBranchInfo::DeserializeAdvertisingMessage(
    boost::uuids::uuid* uuid, unsigned short* tcp_port,
    const utils::ByteVector& adv_msg) {
  YOGI_ASSERT(adv_msg.size() >= kAdvertisingMessageSize);

  auto res = CheckMagicPrefixAndVersion(adv_msg);
  if (res.IsError()) {
    return res;
  }

  auto it = adv_msg.cbegin() + 7;
  network::Deserialize(uuid, adv_msg, &it);
  network::Deserialize(tcp_port, adv_msg, &it);

  return api::kSuccess;
}

api::Result RemoteBranchInfo::DeserializeInfoMessageBodySize(
    std::size_t* body_size, const utils::ByteVector& info_msg_hdr) {
  YOGI_ASSERT(info_msg_hdr.size() >= kInfoMessageHeaderSize);

  auto res = CheckMagicPrefixAndVersion(info_msg_hdr);
  if (res.IsError()) {
    return res;
  }

  auto it = info_msg_hdr.cbegin() + kAdvertisingMessageSize;
  network::Deserialize(body_size, info_msg_hdr, &it);

  return api::kSuccess;
}

api::Result RemoteBranchInfo::CheckMagicPrefixAndVersion(
    const utils::ByteVector& adv_msg) {
  YOGI_ASSERT(adv_msg.size() >= kAdvertisingMessageSize);
  if (std::memcmp(adv_msg.data(), "YOGI", 5)) {
    return api::Error(YOGI_ERR_INVALID_MAGIC_PREFIX);
  }

  if (adv_msg[5] != api::kVersionMajor || adv_msg[6] != api::kVersionMinor) {
    return api::Error(YOGI_ERR_INCOMPATIBLE_VERSION);
  }

  return api::kSuccess;
}

}  // namespace detail
}  // namespace objects

std::ostream& operator<<(std::ostream& os,
                         const objects::detail::BranchInfoPtr& info) {
  if (info) {
    os << '[' << info->GetUuid() << ']';
  } else {
    os << "[????????-????-????-????-????????????]";
  }

  return os;
}

std::ostream& operator<<(std::ostream& os,
                         const objects::detail::LocalBranchInfoPtr& info) {
  return os << std::static_pointer_cast<objects::detail::BranchInfo>(info);
}

std::ostream& operator<<(std::ostream& os,
                         const objects::detail::RemoteBranchInfoPtr& info) {
  return os << std::static_pointer_cast<objects::detail::BranchInfo>(info);
}
