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
namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

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

  if (ifs_.empty() && !use_all_ifs_) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "No matching network interfaces found";
  }

  UpdateLoggingPrefix();
  SetupAcceptors();
}

TcpListener::~TcpListener() {
  for (auto& acc : acceptors_) {
    boost::system::error_code ec;
    acc.close(ec);
  }
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

  YOGI_ASSERT(!acceptors_.empty());
}

void TcpListener::CreateAcceptorForAll() {
  AddAcceptor(
      tcp::endpoint(ip_version_ == utils::IpVersion::k4 ? tcp::v4() : tcp::v6(),
                    static_cast<unsigned short>(port_)));
}

void TcpListener::CreateAcceptorsForSpecific() {
  for (auto& info : ifs_) {
    for (auto& addr : info.addresses) {
      AddAcceptor(tcp::endpoint(addr, static_cast<unsigned short>(port_)));
    }
  }
}

void TcpListener::AddAcceptor(tcp::endpoint ep) {
  boost::system::error_code ec;
  tcp::acceptor acc(asio::make_strand(context_->IoContext()));

  acc.open(ep.protocol(), ec);
  ThrowIfError(ec, YOGI_ERR_OPEN_SOCKET_FAILED, ep.address());

  acc.bind(ep, ec);
  ThrowIfError(ec, YOGI_ERR_BIND_SOCKET_FAILED, ep.address());

  port_ = static_cast<int>(acc.local_endpoint().port());
  acceptors_.push_back(std::move(acc));
}

void TcpListener::ThrowIfError(const boost::system::error_code& ec,
                               int error_code, const ip::address& addr) {
  if (!ec) return;

  api::DescriptiveError e(error_code);
  if (error_code != YOGI_ERR_OPEN_SOCKET_FAILED) {
    e << addr;
    if (port_) e << ":" << port_;
    e << ": ";
  }
  e << ec.message();
  throw e;
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
  for (auto& acc : acceptors_) {
    boost::system::error_code ec;
    acc.listen(acc.max_listen_connections, ec);
    ThrowIfError(ec, YOGI_ERR_LISTEN_SOCKET_FAILED,
                 acc.local_endpoint().address());
    LOG_IFO("Listening for connections on " << acc.local_endpoint().address());
  }
}

void TcpListener::StartAccept(tcp::acceptor* acc) {
  auto weak_self = MakeWeakPtr();
  acc->async_accept(asio::make_strand(context_->IoContext()),
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
  } else if (ec != asio::error::operation_aborted) {
    LOG_ERR("Accepting connection on " << acc->local_endpoint().address()
                                       << " failed: " << ec.message());
  }

  StartAccept(acc);
}

}  // namespace network
