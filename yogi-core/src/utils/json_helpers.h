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

#include "../config.h"

#include <boost/asio/ip/udp.hpp>
#include <nlohmann/json.hpp>

namespace utils {

std::chrono::nanoseconds ExtractDuration(const nlohmann::json& json,
                                         const char* key,
                                         long long defaultValue);
std::vector<std::string> ExtractArrayOfStrings(const nlohmann::json& json,
                                               const char* key,
                                               const char* default_val);

int ExtractLimitedInt(const nlohmann::json& json, const char* key,
                      int default_val, int min_val, int max_val);

std::size_t ExtractSizeWithInfSupport(const nlohmann::json& json,
                                      const char* key, int default_val,
                                      int min_val);

template <typename T>
T ExtractLimitedNumber(const nlohmann::json& json, const char* key,
                       int default_val, int min_val, int max_val) {
  return static_cast<T>(
      ExtractLimitedInt(json, key, default_val, min_val, max_val));
}

boost::asio::ip::udp::endpoint ExtractUdpEndpoint(
    const nlohmann::json& json, const char* addr_key,
    const std::string& default_addr, const char* port_key, int default_port);

template <typename T>
void CopyJsonProperty(const nlohmann::json& from_section, const char* key,
                      T&& default_value, nlohmann::json* to_section) {
  (*to_section)[key] = from_section.value(key, std::forward<T>(default_value));
}

}  // namespace utils
