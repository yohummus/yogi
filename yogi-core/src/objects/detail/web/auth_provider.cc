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

#include "auth_provider.h"
#include "../../../api/errors.h"

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {
namespace detail {
namespace web {

AuthProviderPtr AuthProvider::Create(const nlohmann::json& auth_cfg,
                                     const std::string& logging_prefix) {
  if (!auth_cfg.is_object()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid authentication section.";
  }

  AuthProviderPtr auth;

  auto provider = auth_cfg.value("provider", std::string{});
  if (provider == "config") {
    auth = std::make_unique<ConfigAuthProvider>(auth_cfg);
  } else if (provider == "files") {
    auth = std::make_unique<FilesAuthProvider>(auth_cfg);
  } else {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid authentication provider. Valid types are "
           "\"config\" or \"files\".";
  }

  auth->SetLoggingPrefix(logging_prefix);
  return auth;
}

UserPtr AuthProvider::GetUserOptional(const std::string& username) const {
  return {};
}

AuthProvider::AuthProvider(const nlohmann::json& auth_cfg)
    : readonly_(auth_cfg.value("readonly", false)) {}

ConfigAuthProvider::ConfigAuthProvider(const nlohmann::json& auth_cfg)
    : AuthProvider(auth_cfg) {}

FilesAuthProvider::FilesAuthProvider(const nlohmann::json& auth_cfg)
    : AuthProvider(auth_cfg) {}

}  // namespace web
}  // namespace detail
}  // namespace objects
