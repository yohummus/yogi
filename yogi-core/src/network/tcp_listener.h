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

#pragma once

#include "../config.h"
#include "../utils/system.h"
#include "../objects/log/logger.h"
#include "../objects/context.h"

#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace network {

class TcpListener;
typedef std::shared_ptr<TcpListener> TcpListenerPtr;
typedef std::weak_ptr<TcpListener> TcpListenerWeakPtr;

class TcpListener : public std::enable_shared_from_this<TcpListener>,
                    public objects::log::LoggerUser {
 public:
  typedef std::function<void(boost::asio::ip::tcp::socket socket)> AcceptFn;

  TcpListener(objects::ContextPtr context,
              const std::vector<std::string>& interfaces,
              utils::IpVersion ip_version, int port,
              const std::string& owner_type);

  TcpListener(objects::ContextPtr context,
              const std::vector<std::string>& interfaces,
              utils::IpVersion ip_version, const std::string& owner_type)
      : TcpListener(context, interfaces, ip_version, 0, owner_type) {}

  int GetPort() const { return port_; }
  const utils::NetworkInterfaceInfosVector& GetInterfaces() { return ifs_; }

  void Start(AcceptFn accept_fn);

 private:
  TcpListenerWeakPtr MakeWeakPtr() { return {shared_from_this()}; }
  void UpdateLoggingPrefix();
  void SetupAcceptors();
  void CreateAcceptorForAll();
  void CreateAcceptorsForSpecific();
  void ThrowOpenError(const boost::system::error_code& ec);
  void ThrowBindError(const boost::system::error_code& ec);
  bool CheckAndLogOpenError(const boost::system::error_code& ec,
                            const boost::asio::ip::address& addr);
  bool CheckAndLogBindError(const boost::system::error_code& ec,
                            const boost::asio::ip::address& addr);
  void SetOptionReuseAddr(bool on);
  void SetOptionV6Only(bool on);
  void ListenOnAllAcceptors();
  void StartAccept(boost::asio::ip::tcp::acceptor* acc);
  void OnAcceptFinished(boost::system::error_code ec,
                        boost::asio::ip::tcp::socket socket,
                        boost::asio::ip::tcp::acceptor* acc);

  const objects::ContextPtr context_;
  const utils::IpVersion ip_version_;
  const std::string owner_type_;
  AcceptFn accept_fn_;
  utils::NetworkInterfaceInfosVector ifs_;
  bool use_all_ifs_;
  int port_;
  std::vector<boost::asio::ip::tcp::acceptor> acceptors_;
};

}  // namespace network
