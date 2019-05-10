/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
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

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <string>
#include <vector>

inline std::chrono::nanoseconds ConvertDuration(long long duration) {
  return duration == -1 ? std::chrono::nanoseconds::max()
                        : std::chrono::nanoseconds(duration);
}

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

template <typename Enum>
inline Enum ConvertFlags(int flags, Enum default_flags) {
  return flags ? static_cast<Enum>(flags) : default_flags;
}

bool IsExactlyOneBitSet(int bit_field);
bool IsTimeFormatValid(const std::string& fmt);
bool IsLogFormatValid(std::string fmt);
void CopyUuidToUserBuffer(const boost::uuids::uuid& uuid, void* buffer);
bool CopyStringToUserBuffer(const std::string& str, char* buffer,
                            int buffer_size);

nlohmann::json ParseBranchProps(const char* props, const char* section);
