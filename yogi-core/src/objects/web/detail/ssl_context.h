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

#include "../../../config.h"
#include "../../log/logger.h"

#include <nlohmann/json.hpp>
#include <boost/asio/ssl.hpp>

#include <memory>

namespace objects {
namespace web {
namespace detail {

class SslContext;
typedef std::shared_ptr<SslContext> SslContextPtr;

class SslContext : public log::LoggerUser {
 public:
  static SslContextPtr Create(const nlohmann::json& cfg,
                              const std::string& logging_prefix);

  SslContext(const nlohmann::json& cfg, const std::string& logging_prefix);

  boost::asio::ssl::context& AsioSslContext() { return ssl_ctx_; }
  const std::string& GetPrivateKey() const { return private_key_; }
  const std::string& GetPrivateKeyPassword() const { return private_key_pw_; }
  const std::string& GetCertificateChain() const { return certificate_chain_; }
  const std::string& GetDhParams() const { return dh_params_; }

 private:
  std::string ExtractFromConfigOrFile(const nlohmann::json& ssl_cfg,
                                      const std::string& key,
                                      const char* default_val,
                                      const char* desc);
  std::string LoadFromFile(const nlohmann::json::const_iterator& prop_file,
                           const char* desc);
  std::string LoadFromConfig(const nlohmann::json::const_iterator& prop,
                             const char* desc);
  std::string LoadFromDefault(const char* default_val, const char* desc);
  void CheckPemFormat(const std::string& content, const char* desc,
                      const std::string& source);
  void WarnIfUsingDefaultPrivateKey();
  void CheckPrivateKeyPasswordGiven();
  void SetupSslContext();

  boost::asio::ssl::context ssl_ctx_;
  std::string private_key_;
  std::string private_key_pw_;
  std::string certificate_chain_;
  std::string dh_params_;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
