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

#include "../../../config.h"
#include "text_based_log_sink.h"

#include <fstream>

namespace objects {
namespace detail {

class FileLogSink : public TextBasedLogSink {
 public:
  FileLogSink(api::Verbosity verbosity, std::string filename,
              std::string time_fmt, std::string fmt);

  const std::string& GetGeneratedFilename() const { return filename_; }

 protected:
  virtual void WritePartialOutput(const std::string& str) override;
  virtual void Flush() override;

 private:
  const std::string filename_;
  std::ofstream file_;
};

typedef std::unique_ptr<FileLogSink> FileLogSinkPtr;

}  // namespace detail
}  // namespace objects
