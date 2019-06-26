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

#include "ssl_context.h"
#include "../../../api/errors.h"
#include "../../../api/constants.h"
#include "../../../schema/schema.h"

#include <boost/filesystem.hpp>
#include <fstream>

YOGI_DEFINE_INTERNAL_LOGGER("WebServer")

namespace objects {
namespace web {
namespace detail {

SslContextPtr SslContext::Create(const nlohmann::json& cfg,
                                 const std::string& logging_prefix) {
  return std::make_shared<SslContext>(cfg, logging_prefix);
}

SslContext::SslContext(const nlohmann::json& cfg,
                       const std::string& logging_prefix)
    : ssl_ctx_(boost::asio::ssl::context::tlsv12) {
  schema::ValidateJson(cfg, "web_ssl.schema.json");
  static const nlohmann::json dummy = nlohmann::json::object_t{};
  const auto& ssl_cfg = cfg.value("ssl", dummy);

  SetLoggingPrefix(logging_prefix);

  private_key_ = ExtractFromConfigOrFile(
      ssl_cfg, "private_key", api::kDefaultSslPrivateKey, "private key");
  WarnIfUsingDefaultPrivateKey();

  private_key_pw_ = ssl_cfg.value("private_key_password", "");
  CheckPrivateKeyPasswordGiven();

  certificate_chain_ = ExtractFromConfigOrFile(ssl_cfg, "certificate_chain",
                                               api::kDefaultSslCertificateChain,
                                               "certificate chain");

  dh_params_ = ExtractFromConfigOrFile(
      ssl_cfg, "dh_params", api::kDefaultSslDhParams, "DH parameters");

  SetupSslContext();
}

std::string SslContext::ExtractFromConfigOrFile(const nlohmann::json& ssl_cfg,
                                                const std::string& key,
                                                const char* default_val,
                                                const char* desc) {
  auto prop = ssl_cfg.find(key);
  auto prop_file = ssl_cfg.find(key + "_file");

  if (prop_file != ssl_cfg.end()) {
    return LoadFromFile(prop_file, desc);
  } else if (prop != ssl_cfg.end()) {
    return LoadFromConfig(prop, desc);
  } else {
    return LoadFromDefault(default_val, desc);
  }
}

std::string SslContext::LoadFromFile(
    const nlohmann::json::const_iterator& prop_file, const char* desc) {
  auto filename =
      boost::filesystem::absolute(prop_file->get<std::string>()).string();

  std::ifstream ifs(filename);
  if (!ifs.is_open() || ifs.fail()) {
    throw api::DescriptiveError(YOGI_ERR_READ_FILE_FAILED)
        << "The " << desc << " file " << filename
        << " does not exist or is not readable.";
  }

  auto content = std::string((std::istreambuf_iterator<char>(ifs)),
                             (std::istreambuf_iterator<char>()));

  CheckPemFormat(content, desc, filename);

  LOG_IFO("Using SSL " << desc << " from " << filename);
  return content;
}

std::string SslContext::LoadFromConfig(
    const nlohmann::json::const_iterator& prop, const char* desc) {
  std::string content;

  if (prop->is_string()) {
    content = prop->get<std::string>();
  } else {
    std::stringstream ss;
    for (auto& line : *prop) {
      ss << line.get<std::string>() << '\n';
    }

    content = ss.str();
  }

  CheckPemFormat(content, desc, "the configuration");

  LOG_IFO("Using SSL " << desc << " from configuration");
  return content;
}

std::string SslContext::LoadFromDefault(const char* default_val,
                                        const char* desc) {
  LOG_IFO("Using default SSL " << desc);
  return default_val;
}

void SslContext::CheckPemFormat(const std::string& content, const char* desc,
                                const std::string& source) {
  if (content.find("-----BEGIN ") == std::string::npos) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "The SSL " << desc << " supplied in " << source
        << " is not in PEM format.";
  }
}

void SslContext::WarnIfUsingDefaultPrivateKey() {
  if (private_key_ == api::kDefaultSslPrivateKey) {
    LOG_WRN(
        "Using the default SSL private key is unsafe since the key is publicly "
        "available! Please supply your own SSL key and certificate.");
  }
}

void SslContext::CheckPrivateKeyPasswordGiven() {
  if (private_key_.find("ENCRYPTED") != std::string::npos &&
      private_key_pw_.empty()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "The SSL private key is encrypted but no password has been supplied "
           "in the configuration.";
  }
}

void SslContext::SetupSslContext() {
  boost::system::error_code ec;
  ssl_ctx_.set_password_callback(
      [this](auto, auto) { return this->private_key_pw_; }, ec);
  if (ec) LOG_ERR("Coult not set SSL password callback: " << ec.message());

  ssl_ctx_.set_options(boost::asio::ssl::context::default_workarounds |
                           boost::asio::ssl::context::no_sslv2 |
                           boost::asio::ssl::context::single_dh_use,
                       ec);
  if (ec) LOG_ERR("Could not set SSL options: " << ec.message());

  ssl_ctx_.use_certificate_chain(
      boost::asio::buffer(certificate_chain_, certificate_chain_.size()), ec);
  if (ec) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Setting SSL certificate chain failed: " << ec.message();
  }

  ssl_ctx_.use_private_key(
      boost::asio::buffer(private_key_.data(), private_key_.size()),
      boost::asio::ssl::context::file_format::pem, ec);
  if (ec) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Setting SSL private key failed: " << ec.message();
  }

  ssl_ctx_.use_tmp_dh(boost::asio::buffer(dh_params_.data(), dh_params_.size()),
                      ec);
  if (ec) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Setting DH parameters failed: " << ec.message();
  }
}

}  // namespace detail
}  // namespace web
}  // namespace objects
