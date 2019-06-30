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

#include "../../common.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;
namespace asio = boost::asio;
using asio::ip::tcp;

class WebServerTest : public TestFixture {
 protected:
  void* context_ = CreateContext();
  void* server_ = nullptr;
  int port_ = 0;

  void CreateServer(nlohmann::json json = nlohmann::json::object_t{},
                    const char* section = nullptr, void* branch = nullptr) {
    port_ = FindUnusedPort();

    json["port"] = port_;
    void* config = MakeConfigFromJson(json);

    server_ = CreateWebServer(context_, branch, config, section);
  }
};

TEST_F(WebServerTest, ConstructWithoutBranch) { CreateServer(); }

TEST_F(WebServerTest, HttpRedirect) {
  CreateServer();
  RunContextInBackground(context_);

  auto resp = DoHttpRequest(port_, YOGI_MET_GET, "/", {}, false);
  EXPECT_EQ(resp.result_int(), 301);
  EXPECT_EQ(resp[http::field::location].find("https://"), 0);
  EXPECT_FALSE(resp.keep_alive());
}

TEST_F(WebServerTest, WelcomePage) {
  CreateServer();
  RunContextInBackground(context_);

  auto resp = DoHttpRequest(port_, YOGI_MET_GET, "/");
  EXPECT_EQ(resp.result_int(), 200);
  EXPECT_NE(resp.body().find("Welcome to the Yogi web server"),
            std::string::npos)
      << resp.body();
}

TEST_F(WebServerTest, ConnectionKeepAlive) {
  CreateServer();
  RunContextInBackground(context_);

  auto resp = DoHttpRequest(port_, YOGI_MET_GET, "/");
  EXPECT_TRUE(resp.keep_alive());
}

TEST_F(WebServerTest, CleanDestruction) {
  CreateServer();
  RunContextInBackground(context_);

  auto ep = MakeWebServerEndpoint(port_);

  // Connect without sending anything => keeps session on server running
  asio::io_context ioc;
  tcp::socket socket(ioc);
  socket.connect(ep);

  // Do a proper request to make sure that the previous session started
  DoHttpRequest(ep, YOGI_MET_GET, "/");

  int res = YOGI_DestroyAll();
  EXPECT_OK(res)
      << "A server session seems to keep some Yogi objects alive, probably by "
         "holding active shared_ptr's when it shouldn't.";
}
