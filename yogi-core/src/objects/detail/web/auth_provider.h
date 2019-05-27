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
#include "../../logger.h"
#include "group.h"
#include "user.h"

#include <nlohmann/json.hpp>
#include <memory>

namespace objects {
namespace detail {
namespace web {

class AuthProvider;
class ConfigAuthProvider;
class FilesAuthProvider;

typedef std::unique_ptr<AuthProvider> AuthProviderPtr;

class AuthProvider : public objects::LoggerUser {
 public:
  static AuthProviderPtr Create(const nlohmann::json& auth_cfg,
                                const std::string& logging_prefix);

  virtual ~AuthProvider() {}

  bool IsReadonly() const { return readonly_; }
  UserPtr GetUserOptional(const std::string& username) const;

 protected:
  AuthProvider(const nlohmann::json& auth_cfg);

 private:
  const bool readonly_;
};

class ConfigAuthProvider : public AuthProvider {
 public:
  ConfigAuthProvider(const nlohmann::json& auth_cfg);
};

class FilesAuthProvider : public AuthProvider {
 public:
  FilesAuthProvider(const nlohmann::json& auth_cfg);
};

}  // namespace web
}  // namespace detail
}  // namespace objects
