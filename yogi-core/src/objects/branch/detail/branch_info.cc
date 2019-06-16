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

#include "branch_info.h"
#include "../../../api/constants.h"
#include "../../../network/ip.h"
#include "../../../network/serialize.h"
#include "../../../utils/json_helpers.h"
#include "../../../utils/system.h"

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <string>
using namespace std::string_literals;

namespace objects {
namespace branch {
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

  auto uuid_str = boost::uuids::to_string(uuid_);
  SetLoggingPrefix("["s + uuid_str + ']');

  json_ = {
      {"uuid", uuid_str},
      {"name", name_},
      {"description", description_},
      {"network_name", net_name_},
      {"path", path_},
      {"hostname", hostname_},
      {"pid", pid_},
      {"tcp_server_port", tcp_server_port_},
      {"start_time", start_time_.ToJavaScriptString()},
      {"timeout", timeout},
      {"advertising_interval", adv_interval},
      {"ghost_mode", ghost_mode_},
  };
}

LocalBranchInfo::LocalBranchInfo(
    const nlohmann::json& cfg,
    const utils::NetworkInterfaceInfosVector& adv_ifs,
    unsigned short tcp_server_port) {
  // clang-format off
  uuid_            = boost::uuids::random_generator()();
  name_            = cfg.value("name", std::to_string(utils::GetProcessId()) + '@' + utils::GetHostname());
  description_     = cfg.value("description", std::string{});
  net_name_        = cfg.value("network_name", utils::GetHostname());
  path_            = cfg.value("path", "/"s + name_);
  hostname_        = utils::GetHostname();
  pid_             = utils::GetProcessId();
  adv_ifs_         = adv_ifs;
  tcp_server_port_ = tcp_server_port;
  start_time_      = utils::Timestamp::Now();
  timeout_         = utils::ExtractDuration(cfg, "timeout", api::kDefaultConnectionTimeout);
  adv_interval_    = utils::ExtractDuration(cfg, "advertising_interval", api::kDefaultAdvInterval);
  ghost_mode_      = cfg.value("ghost_mode", false);
  adv_ep_          = utils::ExtractUdpEndpoint(cfg, "advertising_address", api::kDefaultAdvAddress, "advertising_port", api::kDefaultAdvPort);
  tx_queue_size_   = utils::ExtractSize(cfg, "tx_queue_size", api::kDefaultTxQueueSize);
  rx_queue_size_   = utils::ExtractSize(cfg, "rx_queue_size", api::kDefaultRxQueueSize);
  txrx_byte_limit_ = utils::ExtractSizeWithInfSupport(cfg, "_transceive_byte_limit", -1);
  // clang-format on

  PopulateMessages();
  PopulateJson();
  PopulateJsonWithLocalInfo();
}

void LocalBranchInfo::PopulateMessages() {
  utils::ByteVector buffer{'Y', 'O', 'G', 'I', 0};
  buffer.push_back(api::kVersionMajor);
  buffer.push_back(api::kVersionMinor);
  network::Serialize(&buffer, uuid_);
  network::Serialize(&buffer, tcp_server_port_);

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
  auto res = DeserializeAdvertisingMessage(&uuid_, &tcp_server_port_, info_msg);
  if (res.IsError()) {
    throw res.ToError();
  }

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

  auto addr_str = addr.to_string();
  addr_str = addr_str.substr(0, addr_str.find('%'));
  json_["tcp_server_address"] = addr_str;
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

  if (adv_msg[5] != api::kVersionMajor) {
    return api::Error(YOGI_ERR_INCOMPATIBLE_VERSION);
  }

  return api::kSuccess;
}

}  // namespace detail
}  // namespace branch
}  // namespace objects

std::ostream& operator<<(std::ostream& os,
                         const objects::branch::detail::BranchInfoPtr& info) {
  if (info) {
    os << '[' << info->GetUuid() << ']';
  } else {
    os << "[????????-????-????-????-????????????]";
  }

  return os;
}

std::ostream& operator<<(
    std::ostream& os, const objects::branch::detail::LocalBranchInfoPtr& info) {
  return os << std::static_pointer_cast<objects::branch::detail::BranchInfo>(
             info);
}

std::ostream& operator<<(
    std::ostream& os,
    const objects::branch::detail::RemoteBranchInfoPtr& info) {
  return os << std::static_pointer_cast<objects::branch::detail::BranchInfo>(
             info);
}
