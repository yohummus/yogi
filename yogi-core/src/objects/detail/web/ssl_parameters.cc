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

#include "ssl_parameters.h"
#include "../../../api/errors.h"
#include "../../../api/constants.h"
#include "../../../utils/schema.h"

#include <boost/filesystem.hpp>
#include <fstream>

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {
namespace detail {
namespace web {

SslParameters::SslParameters(const nlohmann::json& cfg,
                             const std::string& logging_prefix) {
  utils::ValidateJson(cfg, "web_ssl.schema.json");
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
}

std::string SslParameters::ExtractFromConfigOrFile(
    const nlohmann::json& ssl_cfg, const std::string& key,
    const char* default_val, const char* desc) {
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

std::string SslParameters::LoadFromFile(
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

std::string SslParameters::LoadFromConfig(
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

std::string SslParameters::LoadFromDefault(const char* default_val,
                                           const char* desc) {
  LOG_IFO("Using default SSL " << desc);
  return default_val;
}

void SslParameters::CheckPemFormat(const std::string& content, const char* desc,
                                   const std::string& source) {
  if (content.find("-----BEGIN ") == std::string::npos) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "The SSL " << desc << " supplied in " << source
        << " is not in PEM format.";
  }
}

void SslParameters::WarnIfUsingDefaultPrivateKey() {
  if (private_key_ == api::kDefaultSslPrivateKey) {
    LOG_WRN(
        "Using the default SSL private key is unsafe since the key is publicly "
        "available! Please supply your own SSL key and certificate.");
  }
}

void SslParameters::CheckPrivateKeyPasswordGiven() {
  if (private_key_.find("ENCRYPTED") != std::string::npos &&
      private_key_pw_.empty()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "The SSL private key is encrypted but no password has been supplied "
           "in the configuration.";
  }
}

}  // namespace web
}  // namespace detail
}  // namespace objects
