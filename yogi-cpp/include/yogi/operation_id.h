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

#ifndef YOGI_OPERATION_ID_H
#define YOGI_OPERATION_ID_H

//! \file
//!
//! Classes for handling IDs for asynchronous operations.

namespace yogi {

class OperationId;

namespace internal {

inline OperationId MakeOperationId(int val);

}  // namespace internal

////////////////////////////////////////////////////////////////////////////////
/// Represents the ID of an asynchronous operation.
////////////////////////////////////////////////////////////////////////////////
class OperationId {
  friend OperationId internal::MakeOperationId(int);

 public:
  /// Constructs an invalid operation ID.
  OperationId() : id_(0) {}

  /// Returns the numerical value of the ID.
  ///
  /// \returns Numerical value of the ID.
  int Value() const { return id_; }

  /// Returns true if the operation ID is valid.
  ///
  /// \returns True if the operation ID is valid.
  bool IsValid() const { return id_ > 0; }

  bool operator==(const OperationId& rhs) const { return id_ == rhs.id_; }
  bool operator!=(const OperationId& rhs) const { return id_ != rhs.id_; }
  bool operator<(const OperationId& rhs) const { return id_ < rhs.id_; }
  bool operator<=(const OperationId& rhs) const { return id_ <= rhs.id_; }
  bool operator>(const OperationId& rhs) const { return id_ > rhs.id_; }
  bool operator>=(const OperationId& rhs) const { return id_ >= rhs.id_; }

 private:
  OperationId(int val) : id_(val) {}

  int id_;
};

namespace internal {

inline OperationId MakeOperationId(int val) { return OperationId(val); }

}  // namespace internal
}  // namespace yogi

#endif  // YOGI_OPERATION_ID_H
