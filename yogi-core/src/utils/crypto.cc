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

#include "crypto.h"
#include "../objects/logger.h"

#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <random>
#include <limits>
#include <mutex>

namespace utils {

const auto logger = objects::Logger::CreateStaticInternalLogger("Utils.Crypto");

ByteVector MakeSha256(const ByteVector& data) {
  ByteVector hash(SHA256_DIGEST_LENGTH);

  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, data.data(), data.size());
  SHA256_Final(hash.data(), &sha256);

  return hash;
}

ByteVector GenerateRandomBytes(std::size_t n) {
  ByteVector bytes(n);

  static std::mutex mutex;  // For libcrypto's RAND_bytes() and error info
  std::lock_guard<std::mutex> lock(mutex);
  auto res = RAND_bytes(bytes.data(), static_cast<int>(bytes.size()));
  if (res != 1) {
    char str[128];
    ERR_error_string_n(ERR_get_error(), str, sizeof(str));
    YOGI_LOG_WARNING(
        logger,
        "Could not generate "
            << n
            << " random bytes using OpenSSL. Generated from C++ STL instead.");

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_int_distribution<> dist;
    for (auto& byte : bytes) {
      byte = static_cast<Byte>(dist(gen) % 256);
    }
  }

  return bytes;
}

}  // namespace utils
