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

#include <boost/filesystem.hpp>
#include <sstream>

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {
namespace detail {
namespace web {

SslParameters::SslParameters(const nlohmann::json& ssl_cfg,
                             const std::string& logging_prefix) {
  if (!ssl_cfg.is_object()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid SSL section.";
  }

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
  CheckUnambiguousSource(ssl_cfg, prop, prop_file);

  if (prop_file != ssl_cfg.end()) {
    return LoadFromFile(prop_file, desc);
  } else if (prop != ssl_cfg.end()) {
    return LoadFromConfig(prop, desc);
  } else {
    return LoadFromDefault(default_val, desc);
  }
}

void SslParameters::CheckUnambiguousSource(
    const nlohmann::json& ssl_cfg, const nlohmann::json::const_iterator& prop,
    const nlohmann::json::const_iterator& prop_file) {
  if (prop != ssl_cfg.end() && prop_file != ssl_cfg.end()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "The two properties \"" << prop.key() << "\" and "
        << "\"" << prop_file.key() << "\" cannot be set at the same time.";
  }
}

std::string SslParameters::LoadFromFile(
    const nlohmann::json::const_iterator& prop_file, const char* desc) {
  if (!prop_file->is_string()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "The property \"" << prop_file.key() << "\" must be a string.";
  }

  auto filename =
      boost::filesystem::absolute(prop_file->get<std::string>()).string();

  std::ifstream ifs(filename);
  if (ifs.fail()) {
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
  bool ok = true;
  if (prop->is_string()) {
    content = prop->get<std::string>();
  } else if (prop->is_array()) {
    std::stringstream ss;
    for (auto& line : *prop) {
      if (line.is_string()) {
        ss << line.get<std::string>() << std::endl;
      } else {
        ok = false;
        break;
      }
    }

    content = ss.str();
  } else {
    ok = false;
  }

  if (!ok) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "The property \"" << prop.key()
        << "\" must be either a string or an array of strings.";
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
