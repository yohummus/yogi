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
#include "../objects/log/logger.h"
#include "transport.h"

#include <boost/asio.hpp>

namespace network {

class TcpTransport;
typedef std::shared_ptr<TcpTransport> TcpTransportPtr;
typedef std::weak_ptr<TcpTransport> TcpTransportWeakPtr;

class TcpTransport : public Transport {
 public:
  template <typename T>
  class AsioGuard {
    friend class TcpTransport;

   public:
    AsioGuard(T* obj) : obj_(obj) {}
    ~AsioGuard() {
      if (obj_) obj_->cancel();
    }

   private:
    void Disable() { obj_ = nullptr; }

    T* obj_;
  };

  typedef AsioGuard<boost::asio::ip::tcp::acceptor> AcceptGuard;
  typedef std::shared_ptr<AcceptGuard> AcceptGuardPtr;
  typedef std::weak_ptr<AcceptGuard> AcceptGuardWeakPtr;
  typedef AsioGuard<boost::asio::ip::tcp::socket> ConnectGuard;
  typedef std::shared_ptr<ConnectGuard> ConnectGuardPtr;
  typedef std::weak_ptr<ConnectGuard> ConnectGuardWeakPtr;
  typedef std::function<void(const api::Result&, TcpTransportPtr,
                             AcceptGuardPtr)>
      AcceptHandler;
  typedef std::function<void(const api::Result&, TcpTransportPtr,
                             ConnectGuardPtr)>
      ConnectHandler;

  static AcceptGuardPtr AcceptAsync(objects::ContextPtr context,
                                    boost::asio::ip::tcp::acceptor* acceptor,
                                    std::chrono::nanoseconds timeout,
                                    std::size_t transceive_byte_limit,
                                    AcceptHandler handler);

  static ConnectGuardPtr ConnectAsync(objects::ContextPtr context,
                                      const boost::asio::ip::tcp::endpoint& ep,
                                      std::chrono::nanoseconds timeout,
                                      std::size_t transceive_byte_limit,
                                      ConnectHandler handler);

  TcpTransport(objects::ContextPtr context,
               boost::asio::ip::tcp::socket&& socket,
               std::chrono::nanoseconds timeout,
               std::size_t transceive_byte_limit, bool created_via_accept);

  boost::asio::ip::tcp::endpoint GetPeerEndpoint() const {
    return socket_.remote_endpoint();
  }

 protected:
  virtual void WriteSomeAsync(boost::asio::const_buffer data,
                              TransferSomeHandler handler) override;
  virtual void ReadSomeAsync(boost::asio::mutable_buffer data,
                             TransferSomeHandler handler) override;
  virtual void Shutdown() override;

 private:
  static std::string MakePeerDescription(
      const boost::asio::ip::tcp::socket& socket);
  static void CloseSocket(boost::asio::ip::tcp::socket* s);

  void SetNoDelayOption();

  boost::asio::ip::tcp::socket socket_;
};

}  // namespace network
