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

#include "../../../../config.h"
#include "route.h"

#include <boost/filesystem/path.hpp>

namespace objects {
namespace web {
namespace detail {

class FileSystemRoute : public Route {
 public:
  virtual void HandleRequest(const Request& req, const std::string& uri,
                             const HttpsSessionPtr& session,
                             UserPtr user) override;

  const std::string& GetPath() { return path_; }

 protected:
  virtual void ReadConfiguration(
      const nlohmann::json::const_iterator& route_it) override;
  virtual void LogCreation() override;

 private:
  boost::filesystem::path GetFilePathFromUri(const std::string& uri) const;
  const std::string& GetMimeType(const boost::filesystem::path& file) const;

  std::string path_;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
