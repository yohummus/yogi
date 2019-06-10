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

#include <stdexcept>
#include <sstream>

namespace api {

class Error;

class Result {
 public:
  Result() : ec_(YOGI_ERR_UNKNOWN) {}
  explicit Result(int error_code) : ec_(error_code) {}

  bool operator==(const Result& rhs) const { return ec_ == rhs.ec_; }
  bool operator!=(const Result& rhs) const { return ec_ != rhs.ec_; }

  int GetValue() const { return ec_; }
  int GetErrorCode() const { return ec_ > 0 ? 0 : ec_; }
  const char* GetDescription() const;
  bool IsSuccess() const { return ec_ >= 0; }
  bool IsError() const { return ec_ < 0; }
  Error ToError() const;

 private:
  int ec_;
};

namespace {

const auto kSuccess = Result(YOGI_OK);

}  // anonymous namespace

class Error : public Result, public std::exception {
 public:
  explicit Error(int error_code) : Result(error_code) {
    YOGI_ASSERT(error_code < 0);
  }

  int GetErrorCode() const { return GetValue(); }

  virtual const char* what() const noexcept override;
};

class DescriptiveError : public Error {
 public:
  explicit DescriptiveError(int error_code) : Error(error_code) {}

  DescriptiveError(const DescriptiveError& err)
      : Error(err.GetValue()), oss_(err.oss_.str()) {}

  DescriptiveError(DescriptiveError&& err)
      : Error(err.GetValue()), oss_(std::move(err.oss_)) {}

  template <typename T>
  DescriptiveError& operator<<(T&& val) {
    oss_ << std::forward<T>(val);
    return *this;
  }

  std::string GetDetails() const { return oss_.str(); }

 private:
  std::ostringstream oss_;
};

}  // namespace api

std::ostream& operator<<(std::ostream& os, const api::Result& res);
std::ostream& operator<<(std::ostream& os, const api::Error& err);
std::ostream& operator<<(std::ostream& os, const api::DescriptiveError& err);
