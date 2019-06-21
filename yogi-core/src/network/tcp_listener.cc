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

#include "tcp_listener.h"
#include "../utils/algorithm.h"

#include <boost/asio/strand.hpp>
#include <boost/asio/ip/v6_only.hpp>
using namespace boost::asio::ip;

#include <sstream>
using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("TcpListener");

namespace network {

TcpListener::TcpListener(objects::ContextPtr context,
                         const std::vector<std::string>& interfaces,
                         utils::IpVersion ip_version, int port,
                         const std::string& owner_type)
    : context_(context),
      ip_version_(ip_version),
      owner_type_(owner_type),
      port_(port) {
  ifs_ = utils::GetFilteredNetworkInterfaces(interfaces, ip_version_);
  use_all_ifs_ = utils::contains(interfaces, "all");

  UpdateLoggingPrefix();
  SetupAcceptors();
}

void TcpListener::Start(AcceptFn accept_fn) {
  accept_fn_ = accept_fn;
  for (auto& acc : acceptors_) {
    StartAccept(&acc);
  }
}

void TcpListener::UpdateLoggingPrefix() {
  std::stringstream ss;
  ss << "For " << owner_type_;
  if (port_) ss << " on port " << port_;
  SetLoggingPrefix(ss.str());
}

void TcpListener::SetupAcceptors() {
  if (use_all_ifs_) {
    CreateAcceptorForAll();
  } else {
    CreateAcceptorsForSpecific();
  }

  UpdateLoggingPrefix();
  SetOptionReuseAddr(true);
  SetOptionV6Only(ip_version_ != utils::IpVersion::k4);

  ListenOnAllAcceptors();

  if (acceptors_.empty()) {
    LOG_ERR("No network interfaces available for connections");
  }
}

void TcpListener::CreateAcceptorForAll() {
  boost::system::error_code ec;
  tcp::acceptor acc(boost::asio::make_strand(context_->IoContext()));

  auto prot = ip_version_ == utils::IpVersion::k4 ? tcp::v4() : tcp::v6();
  acc.open(prot, ec);
  ThrowOpenError(ec);

  acc.bind(tcp::endpoint(prot, static_cast<unsigned short>(port_)), ec);
  ThrowBindError(ec);

  port_ = static_cast<int>(acc.local_endpoint().port());
  acceptors_.push_back(std::move(acc));
}

void TcpListener::CreateAcceptorsForSpecific() {
  boost::system::error_code ec;

  if (ifs_.empty()) {
    LOG_WRN(
        "No matching network interfaces found. No connections will be "
        "accepted.");
    return;
  }

  for (auto& info : ifs_) {
    for (auto& addr : info.addresses) {
      tcp::acceptor acc(boost::asio::make_strand(context_->IoContext()));

      acc.open(addr.is_v4() ? tcp::v4() : tcp::v6(), ec);
      if (!CheckAndLogOpenError(ec, addr)) continue;

      acc.bind(tcp::endpoint(addr, static_cast<unsigned short>(port_)), ec);
      if (!CheckAndLogBindError(ec, addr)) continue;

      port_ = static_cast<int>(acc.local_endpoint().port());
      acceptors_.push_back(std::move(acc));
    }
  }

  if (acceptors_.empty()) {
    api::DescriptiveError e(YOGI_ERR_BIND_SOCKET_FAILED);
    e << "Could not bind to ";
    if (port_) {
      e << "port " << port_;
    } else {
      e << "any port";
    }
    e << " of any matching network interface";
    throw e;
  }
}

void TcpListener::ThrowOpenError(const boost::system::error_code& ec) {
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }
}

void TcpListener::ThrowBindError(const boost::system::error_code& ec) {
  if (ec) {
    api::DescriptiveError e(YOGI_ERR_BIND_SOCKET_FAILED);
    if (port_) {
      e << "Port: " << port_;
    } else {
      e << "Any port";
    }
    e << "; " << ec.message();
    throw e;
  }
}

bool TcpListener::CheckAndLogOpenError(const boost::system::error_code& ec,
                                       const boost::asio::ip::address& addr) {
  if (ec) {
    LOG_ERR("Could not open socket. Skipping address " << addr << "...");
    return false;
  }

  return true;
}

bool TcpListener::CheckAndLogBindError(const boost::system::error_code& ec,
                                       const boost::asio::ip::address& addr) {
  if (ec) {
    LOG_ERR("Could not bind to address " << addr << ": " << ec.message());

    return false;
  }

  return true;
}

void TcpListener::SetOptionReuseAddr(bool on) {
  for (auto& acc : acceptors_) {
    boost::system::error_code ec;
    acc.set_option(tcp::acceptor::reuse_address(on), ec);
    if (ec) {
      LOG_ERR("Could not set socket option SO_REUSEADDR: " << ec.message());
    }
  }
}

void TcpListener::SetOptionV6Only(bool on) {
  for (auto& acc : acceptors_) {
    boost::system::error_code ec;
    acc.set_option(tcp::acceptor::reuse_address(on), ec);
    if (ec) {
      LOG_ERR("Could not clear socket option IPV6_V6ONLY: " << ec.message());
    }
  }
}

void TcpListener::ListenOnAllAcceptors() {
  auto it = acceptors_.begin();
  while (it != acceptors_.end()) {
    boost::system::error_code ec;
    it->listen(it->max_listen_connections, ec);
    if (ec) {
      LOG_ERR("Could not listen for connections on "
              << it->local_endpoint().address() << ": " << ec.message());
      it = acceptors_.erase(it);
    } else {
      LOG_IFO("Listening for connections on "
              << it->local_endpoint().address());
      ++it;
    }
  }
}

void TcpListener::StartAccept(tcp::acceptor* acc) {
  auto weak_self = MakeWeakPtr();
  acc->async_accept(boost::asio::make_strand(context_->IoContext()),
                    [=](auto ec, auto socket) {
                      auto self = weak_self.lock();
                      if (!self) return;

                      self->OnAcceptFinished(ec, std::move(socket), acc);
                    });
}

void TcpListener::OnAcceptFinished(boost::system::error_code ec,
                                   tcp::socket socket, tcp::acceptor* acc) {
  if (!ec) {
    LOG_DBG("Accepted connection from " << socket.remote_endpoint());
    accept_fn_(std::move(socket));
  } else if (ec != boost::asio::error::operation_aborted) {
    LOG_ERR("Accepting connection on " << acc->local_endpoint().address()
                                       << " failed: " << ec.message());
  }

  StartAccept(acc);
}

}  // namespace network
