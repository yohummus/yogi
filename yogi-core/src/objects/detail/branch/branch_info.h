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

#pragma once

#include "../../../config.h"
#include "../../../api/errors.h"
#include "../../../utils/timestamp.h"
#include "../../../utils/types.h"
#include "../../../utils/system.h"
#include "../../logger.h"

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <string>
#include <chrono>
#include <memory>

namespace objects {
namespace detail {

class BranchInfo : public LoggerUser {
 public:
  enum {
    kAdvertisingMessageSize = 25,
    kInfoMessageHeaderSize = kAdvertisingMessageSize + 4,
  };

  virtual ~BranchInfo() = default;

  const boost::uuids::uuid& GetUuid() const { return uuid_; }
  const std::string& GetName() const { return name_; }
  const std::string& GetDescription() const { return description_; }
  const std::string& GetNetworkName() const { return net_name_; }
  const std::string& GetPath() const { return path_; }
  const std::string& GetHostname() const { return hostname_; }

  int GetPid() const { return pid_; }

  const boost::asio::ip::tcp::endpoint& GetTcpServerEndpoint() const {
    return tcp_ep_;
  }

  const utils::Timestamp& GetStartTime() const { return start_time_; }
  const std::chrono::nanoseconds& GetTimeout() const { return timeout_; }

  const std::chrono::nanoseconds& GetAdvertisingInterval() const {
    return adv_interval_;
  }

  bool GetGhostMode() const { return ghost_mode_; }
  const nlohmann::json& ToJson() const { return json_; }

 protected:
  void PopulateJson();

  boost::uuids::uuid uuid_;
  std::string name_;
  std::string description_;
  std::string net_name_;
  std::string path_;
  std::string hostname_;
  int pid_;
  boost::asio::ip::tcp::endpoint tcp_ep_;
  utils::Timestamp start_time_;
  std::chrono::nanoseconds timeout_;
  std::chrono::nanoseconds adv_interval_;
  bool ghost_mode_;
  nlohmann::json json_;
};

typedef std::shared_ptr<BranchInfo> BranchInfoPtr;

class LocalBranchInfo;
typedef std::shared_ptr<LocalBranchInfo> LocalBranchInfoPtr;

class LocalBranchInfo : public BranchInfo {
 public:
  LocalBranchInfo(const nlohmann::json& cfg,
                  const std::vector<utils::NetworkInterfaceInfo>& adv_ifs,
                  const boost::asio::ip::tcp::endpoint& tcp_ep);

  const std::vector<utils::NetworkInterfaceInfo>& GetAdvertisingInterfaces()
      const {
    return adv_ifs_;
  }

  const boost::asio::ip::udp::endpoint& GetAdvertisingEndpoint() const {
    return adv_ep_;
  }

  std::size_t GetTxQueueSize() const { return tx_queue_size_; }
  std::size_t GetRxQueueSize() const { return rx_queue_size_; }
  std::size_t GetTransceiveByteLimit() const { return txrx_byte_limit_; }

  utils::SharedByteVector MakeAdvertisingMessage() const {
    YOGI_ASSERT(adv_msg_);
    return adv_msg_;
  }

  utils::SharedByteVector MakeInfoMessage() const {
    YOGI_ASSERT(info_msg_);
    return info_msg_;
  };

 private:
  void PopulateMessages();
  void PopulateJsonWithLocalInfo();

  std::vector<utils::NetworkInterfaceInfo> adv_ifs_;
  boost::asio::ip::udp::endpoint adv_ep_;
  std::size_t tx_queue_size_;
  std::size_t rx_queue_size_;
  std::size_t txrx_byte_limit_;
  utils::SharedByteVector adv_msg_;
  utils::SharedByteVector info_msg_;
};

class RemoteBranchInfo;
typedef std::shared_ptr<RemoteBranchInfo> RemoteBranchInfoPtr;

class RemoteBranchInfo : public BranchInfo {
 public:
  RemoteBranchInfo(const utils::ByteVector& info_msg,
                   const boost::asio::ip::address& addr);

  static api::Result DeserializeAdvertisingMessage(
      boost::uuids::uuid* uuid, unsigned short* tcp_port,
      const utils::ByteVector& adv_msg);

  static api::Result DeserializeInfoMessageBodySize(
      std::size_t* body_size, const utils::ByteVector& info_msg_hdr);

 private:
  static api::Result CheckMagicPrefixAndVersion(
      const utils::ByteVector& adv_msg);
};

}  // namespace detail
}  // namespace objects

// format like this: [6ba7b810-9dad-11d1-80b4-00c04fd430c8]
std::ostream& operator<<(std::ostream& os,
                         const objects::detail::BranchInfoPtr& info);
std::ostream& operator<<(std::ostream& os,
                         const objects::detail::LocalBranchInfoPtr& info);
std::ostream& operator<<(std::ostream& os,
                         const objects::detail::RemoteBranchInfoPtr& info);
