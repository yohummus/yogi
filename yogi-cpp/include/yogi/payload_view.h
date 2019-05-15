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

#ifndef YOGI_PAYLOAD_VIEW_H
#define YOGI_PAYLOAD_VIEW_H

//! \file
//!
//! Helpers for passing and converting user payload to functions.

#include "json_view.h"
#include "msgpack_view.h"

namespace yogi {

////////////////////////////////////////////////////////////////////////////////
/// Possible data/payload encoding types.
////////////////////////////////////////////////////////////////////////////////
enum class EncodingType {
  kJson = 0,     ///< Data is encoded as JSON.
  kMsgpack = 1,  ///< Data is encoded as MessagePack
};

////////////////////////////////////////////////////////////////////////////////
/// Helper class for passing and converting different types of user payload.
///
/// \attention
///   It is imperative that the objects passed to any of the view's constructors
///   outlive the view object!
////////////////////////////////////////////////////////////////////////////////
class PayloadView {
 public:
  /// Constructs a view that evaluates to a nullptr.
  PayloadView() : PayloadView(nullptr, 0, EncodingType::kJson) {}

  /// Constructs a view from a buffer.
  ///
  /// \param data Buffer to use.
  PayloadView(const std::vector<char>& data, EncodingType enc)
      : PayloadView(data.data(), data.size(), enc) {}

  /// Constructs a view from a buffer.
  ///
  /// \param data Buffer to use.
  /// \param size Size of the buffer in bytes.
  /// \param enc  Encoding.
  PayloadView(const char* data, int size, EncodingType enc)
      : data_(data), size_(size), enc_(enc) {}

  /// Constructs a view from a buffer.
  ///
  /// \param data Buffer to use.
  /// \param size Size of the buffer in bytes.
  /// \param enc  Encoding.
  PayloadView(const char* data, std::size_t size, EncodingType enc)
      : PayloadView(data, static_cast<int>(size), enc) {}

  /// Constructs a view from JSON data.
  ///
  /// \param json The JSON data.
  PayloadView(const JsonView& json)
      : PayloadView(json.Data(), json.Size(), EncodingType::kJson) {}

  /// Constructs a view from MessagePack data.
  ///
  /// \param msgpack The MessagePack data.
  PayloadView(const MsgpackView& msgpack)
      : PayloadView(msgpack.Data(), msgpack.Size(), EncodingType::kMsgpack) {}

  /// Returns a buffer holding the payload data.
  ///
  /// \attention
  ///   The returned value is only valid as long as both the view object and the
  ///   parameters passed to any of its constructors are valid.
  ///
  /// \returns Buffer holding the payload data.
  const char* Data() const { return data_; }

  /// Returns the size of the payload data in bytes.
  ///
  /// \returns Size of the payload data in bytes.
  int Size() const { return size_; };

  /// Returns the encoding of the payload data.
  ///
  /// \returns Encoding of the payload data.
  EncodingType Encoding() const { return enc_; }

  bool operator==(const PayloadView& rhs) const {
    if ((Encoding() != rhs.Encoding())) return false;
    if (Size() != rhs.Size()) return false;
    return std::memcmp(Data(), rhs.Data(), static_cast<std::size_t>(Size())) ==
           0;
  }

  bool operator!=(const PayloadView& rhs) const { return !(*this == rhs); }

 private:
  const char* data_;
  int size_;
  EncodingType enc_;
};

inline bool operator==(const PayloadView& lhs, const JsonView& rhs) {
  return lhs == PayloadView(rhs);
}

inline bool operator!=(const PayloadView& lhs, const JsonView& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const JsonView& lhs, const PayloadView& rhs) {
  return rhs == lhs;
}

inline bool operator!=(const JsonView& lhs, const PayloadView& rhs) {
  return rhs != lhs;
}

inline bool operator==(const PayloadView& lhs, const MsgpackView& rhs) {
  return lhs == PayloadView(rhs);
}

inline bool operator!=(const PayloadView& lhs, const MsgpackView& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const MsgpackView& lhs, const PayloadView& rhs) {
  return rhs == lhs;
}

inline bool operator!=(const MsgpackView& lhs, const PayloadView& rhs) {
  return rhs != lhs;
}

}  // namespace yogi

#endif  // YOGI_PAYLOAD_VIEW_H
