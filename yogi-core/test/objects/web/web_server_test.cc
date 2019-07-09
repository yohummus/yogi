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

TEST_F(WebServerTest, SchemaValidation) {
  char err[256] = {0};
  void* config = MakeConfigFromJson(nlohmann::json::parse(R"(
    {
      "port": "should be an int"
    }
  )"));

  int res = YOGI_WebServerCreate(&server_, context_, nullptr, config, nullptr,
                                 err, sizeof(err));
  EXPECT_ERR(res, YOGI_ERR_CONFIGURATION_VALIDATION_FAILED);
  EXPECT_NE(err, "");
}

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

TEST_F(WebServerTest, InvalidMethod) {
  CreateServer();
  RunContextInBackground(context_);

  auto resp = DoHttpRequest(port_, YOGI_MET_DELETE, "/");
  EXPECT_EQ(resp.result_int(), 405);
}

TEST_F(WebServerTest, Authentication) {
  CreateServer();
  RunContextInBackground(context_);

  auto resp =
      DoHttpRequest(port_, YOGI_MET_GET, "/", MakeAuthRequestModifierFn());
  EXPECT_LT(resp.result_int(), 400);

  resp = DoHttpRequest(
      port_, YOGI_MET_GET, "/",
      MakeAuthRequestModifierFn(api::kDefaultAdminUser,
                                ""s + api::kDefaultAdminPassword + "bla"));
  EXPECT_EQ(resp.result_int(), 401);

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/", [](auto* req) {
    req->set(http::field::authorization, "Basic aGVsbG93b3JsZA==");
  });
  EXPECT_EQ(resp.result_int(), 401);

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/", [](auto* req) {
    req->set(http::field::authorization, "Hello sdf");
  });
  EXPECT_EQ(resp.result_int(), 401);
}

TEST_F(WebServerTest, ReuseConnection) {
  CreateServer();
  RunContextInBackground(context_);

  auto resp = DoHttpRequest(port_, YOGI_MET_GET, "/", {}, true, 3);
  EXPECT_EQ(resp.result_int(), 200);
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

TEST_F(WebServerTest, HeaderLimit) {
  CreateServer(nlohmann::json::parse(R"(
    {
      "http_header_limit": 200
    }
  )"));
  RunContextInBackground(context_);

  std::string target = "/xxxxx";
  EXPECT_NO_THROW(DoHttpRequest(port_, YOGI_MET_GET, target, {}, true));
  EXPECT_NO_THROW(DoHttpRequest(port_, YOGI_MET_GET, target, {}, false));

  target.resize(250, 'x');
  EXPECT_ANY_THROW(DoHttpRequest(port_, YOGI_MET_GET, target, {}, true));
  EXPECT_ANY_THROW(DoHttpRequest(port_, YOGI_MET_GET, target, {}, false));
}

TEST_F(WebServerTest, BodyLimit) {
  CreateServer(nlohmann::json::parse(R"(
    {
      "http_body_limit": 50
    }
  )"));
  RunContextInBackground(context_);

  std::string body = "xxxxx";
  auto fn = [&](http::request<http::string_body>* req) {
    req->set(http::field::content_type, "text/plain");
    req->body() = body;
    req->prepare_payload();
  };

  EXPECT_NO_THROW(DoHttpRequest(port_, YOGI_MET_GET, "/", fn, true));
  EXPECT_NO_THROW(DoHttpRequest(port_, YOGI_MET_GET, "/", fn, false));

  body.resize(100, 'x');
  EXPECT_ANY_THROW(DoHttpRequest(port_, YOGI_MET_GET, "/", fn, true));
  EXPECT_ANY_THROW(DoHttpRequest(port_, YOGI_MET_GET, "/", fn, false));
}

TEST_F(WebServerTest, Worker) {
  CreateServer();
  RunContextInBackground(context_);

  void* worker = CreateContext();
  int res = YOGI_WebServerAddWorker(server_, worker);
  ASSERT_OK(res);

  std::thread th([&] {
    try {
      DoHttpRequest(this->port_, YOGI_MET_GET, "/");
    } catch (...) {
    }
  });

  res = YOGI_ContextRunOne(worker, nullptr, -1);
  EXPECT_OK(res);

  RunContextInBackground(worker);
  th.join();
}

TEST_F(WebServerTest, ContentRoute) {
  CreateServer();
  RunContextInBackground(context_);

  auto resp = DoHttpRequest(port_, YOGI_MET_GET, "/");
  EXPECT_EQ(resp.result_int(), 200);
  EXPECT_FALSE(resp.body().empty());

  resp = DoHttpRequest(port_, YOGI_MET_HEAD, "/");
  EXPECT_EQ(resp.result_int(), 200);
  EXPECT_TRUE(resp.has_content_length());
}

TEST_F(WebServerTest, ContentRoutePermissions) {
  CreateServer(nlohmann::json::parse(R"(
    {
      "routes": {
        "/allowed": {
          "type": "content",
          "permissions": { "*": ["GET"] },
          "mime": "text/plain",
          "content": "allowed"
        },
        "/denied": {
          "type": "content",
          "permissions": {},
          "mime": "text/plain",
          "content": "denied"
        }
      }
    }
  )"));
  RunContextInBackground(context_);

  auto resp = DoHttpRequest(port_, YOGI_MET_GET, "/allowed");
  EXPECT_EQ(resp.result_int(), 200);
  EXPECT_EQ(resp.body(), "allowed");

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/denied");
  EXPECT_EQ(resp.result_int(), 403);
  EXPECT_NE(resp.body(), "denied");
}

TEST_F(WebServerTest, FileSystemRoute) {
  auto cfg = nlohmann::json::parse(R"(
    {
      "routes": {
        "/foo": {
          "type": "filesystem",
          "path": "",
          "permissions": { "*": ["GET"] }
        },
        "/foo/bar": {
          "type": "filesystem",
          "path": "",
          "permissions": {}
        }
      }
    }
  )");

  cfg["routes"]["/foo"]["path"] = MakeTestDataPath("www");
  cfg["routes"]["/foo/bar"]["path"] = MakeTestDataPath("www/stuff");

  CreateServer(cfg);
  RunContextInBackground(context_);

  auto resp = DoHttpRequest(port_, YOGI_MET_GET, "/foo/");
  EXPECT_EQ(resp.result_int(), 200);
  EXPECT_EQ(resp[http::field::content_type], "text/html");
  EXPECT_NE(resp.body().find("<body>"), std::string::npos);

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/foo");
  EXPECT_EQ(resp.result_int(), 200);
  EXPECT_EQ(resp[http::field::content_type], "text/html");
  EXPECT_NE(resp.body().find("<body>"), std::string::npos);

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/foo/bar/hello.txt");
  EXPECT_EQ(resp.result_int(), 403);

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/foo/bar/hello.txt",
                       MakeAuthRequestModifierFn());
  EXPECT_EQ(resp.result_int(), 200);
  EXPECT_EQ(resp[http::field::content_type], "text/plain");
  EXPECT_EQ(resp.body().find("Hello World!"), 0);

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/foo/bar/../index.txt",
                       MakeAuthRequestModifierFn());
  EXPECT_EQ(resp.result_int(), 400);

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/foo/..");
  EXPECT_EQ(resp.result_int(), 400);

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/foo/public/test.json");
  EXPECT_EQ(resp.result_int(), 200);
  EXPECT_EQ(resp[http::field::content_type], "application/json");

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/foo/public/../public/test.json");
  EXPECT_EQ(resp.result_int(), 400);

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/foo/public/nothing.txt");
  EXPECT_EQ(resp.result_int(), 404);

  resp = DoHttpRequest(port_, YOGI_MET_GET, "/foo/public/");
  EXPECT_EQ(resp.result_int(), 404);
}
