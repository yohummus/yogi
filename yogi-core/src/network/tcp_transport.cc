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

#include "tcp_transport.h"
#include "ip.h"

YOGI_DEFINE_INTERNAL_LOGGER("Transport.Tcp");

namespace network {

TcpTransport::TcpTransport(objects::ContextPtr context,
                           boost::asio::ip::tcp::socket&& socket,
                           std::chrono::nanoseconds timeout,
                           std::size_t transceive_byte_limit,
                           bool created_via_accept)
    : Transport(context, timeout, created_via_accept,
                MakePeerDescription(socket), transceive_byte_limit),
      socket_(std::move(socket)) {}

void TcpTransport::SetNoDelayOption() {
  boost::system::error_code ec;
  std::lock_guard<std::mutex> lock(socket_mutex_);
  socket_.set_option(boost::asio::ip::tcp::no_delay(true), ec);
  if (ec) {
    LOG_WRN("Could not set TCP_NODELAY option on socket: " << ec.message());
  }
}

TcpTransport::AcceptGuardPtr TcpTransport::AcceptAsync(
    objects::ContextPtr context, boost::asio::ip::tcp::acceptor* acceptor,
    std::chrono::nanoseconds timeout, std::size_t transceive_byte_limit,
    AcceptHandler handler) {
  auto guard = std::make_shared<AcceptGuard>(acceptor);
  auto weak_guard = AcceptGuardWeakPtr(guard);
  auto weak_context = context->MakeWeakPtr();

  auto socket =
      std::make_shared<boost::asio::ip::tcp::socket>(context->IoContext());
  acceptor->async_accept(*socket, [=](auto& ec) {
    auto guard = weak_guard.lock();
    if (guard) guard->Disable();

    auto context = weak_context.lock();
    if (!context) return;

    if (!ec) {
      auto transport = TcpTransportPtr(new TcpTransport(
          context, std::move(*socket), timeout, transceive_byte_limit, true));
      transport->SetNoDelayOption();
      handler(api::kSuccess, transport, guard);
    } else if (ec == boost::asio::error::operation_aborted) {
      handler(api::Error(YOGI_ERR_CANCELED), {}, guard);
    } else {
      handler(api::Error(YOGI_ERR_ACCEPT_SOCKET_FAILED), {}, guard);
    }
  });

  return guard;
}

TcpTransport::ConnectGuardPtr TcpTransport::ConnectAsync(
    objects::ContextPtr context, const boost::asio::ip::tcp::endpoint& ep,
    std::chrono::nanoseconds timeout, std::size_t transceive_byte_limit,
    ConnectHandler handler) {
  struct ConnectData {
    ConnectData(boost::asio::io_context& ioc) : socket(ioc), timer(ioc) {}

    boost::asio::ip::tcp::socket socket;
    boost::asio::steady_timer timer;
    bool timed_out = false;
    bool running = true;
  };

  auto condat = std::make_shared<ConnectData>(context->IoContext());

  auto guard = std::make_shared<ConnectGuard>(&condat->socket);
  auto weak_guard = ConnectGuardWeakPtr(guard);
  auto weak_context = context->MakeWeakPtr();

  condat->socket.async_connect(ep, [=](auto& ec) {
    auto guard = weak_guard.lock();
    if (guard) guard->Disable();

    auto context = weak_context.lock();
    if (!context) return;

    condat->running = false;
    condat->timer.cancel();

    if (condat->timed_out) {
      handler(api::Error(YOGI_ERR_TIMEOUT), {}, guard);
    } else if (!ec) {
      auto transport = TcpTransportPtr(
          new TcpTransport(context, std::move(condat->socket), timeout,
                           transceive_byte_limit, false));
      handler(api::kSuccess, transport, guard);
    } else if (ec == boost::asio::error::operation_aborted) {
      handler(api::Error(YOGI_ERR_CANCELED), {}, guard);
    } else {
      handler(api::Error(YOGI_ERR_CONNECT_SOCKET_FAILED), {}, guard);
    }
  });

  condat->timer.expires_from_now(timeout);
  condat->timer.async_wait([=](auto& ec) {
    if (ec) return;

    if (condat->running) {
      condat->timed_out = true;
      CloseSocket(&condat->socket);
    }
  });

  return guard;
}

void TcpTransport::WriteSomeAsync(boost::asio::const_buffer data,
                                  TransferSomeHandler handler) {
  std::lock_guard<std::mutex> lock(socket_mutex_);
  socket_.async_write_some(data, [=](auto& ec, auto bytes_written) {
    if (!ec) {
      handler(api::kSuccess, bytes_written);
    } else if (ec == boost::asio::error::operation_aborted) {
      handler(api::Error(YOGI_ERR_CANCELED), bytes_written);
    } else {
      handler(api::Error(YOGI_ERR_RW_SOCKET_FAILED), bytes_written);
    }
  });
}

void TcpTransport::ReadSomeAsync(boost::asio::mutable_buffer data,
                                 TransferSomeHandler handler) {
  std::lock_guard<std::mutex> lock(socket_mutex_);
  socket_.async_read_some(data, [=](auto& ec, auto bytes_read) {
    if (!ec) {
      handler(api::kSuccess, bytes_read);
    } else if (ec == boost::asio::error::operation_aborted) {
      handler(api::Error(YOGI_ERR_CANCELED), bytes_read);
    } else {
      handler(api::Error(YOGI_ERR_RW_SOCKET_FAILED), bytes_read);
    }
  });
}

void TcpTransport::Shutdown() {
  std::lock_guard<std::mutex> lock(socket_mutex_);
  CloseSocket(&socket_);
}

std::string TcpTransport::MakePeerDescription(
    const boost::asio::ip::tcp::socket& socket) {
  auto ep = socket.remote_endpoint();
  return MakeIpAddressString(ep) + ':' + std::to_string(ep.port());
}

void TcpTransport::CloseSocket(boost::asio::ip::tcp::socket* s) {
  boost::system::error_code ec;
  s->cancel(ec);
  s->shutdown(s->shutdown_both, ec);
  s->close(ec);
}

}  // namespace network
