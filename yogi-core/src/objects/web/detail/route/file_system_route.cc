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

#include "file_system_route.h"
#include "../session/https_session.h"
#include "../session/methods.h"
#include "../session/mime.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
namespace beast = boost::beast;
namespace http = beast::http;
namespace errc = beast::errc;

YOGI_DEFINE_INTERNAL_LOGGER("WebServer.Session.HTTPS")

namespace objects {
namespace web {
namespace detail {

void FileSystemRoute::HandleRequest(const Request& req, const std::string& uri,
                                    const HttpsSessionPtr& session,
                                    UserPtr user) {
  auto method = VerbToMethod(req.method());

  if (method != api::kGet && method != api::kHead) {
    session->SendResponse(http::status::method_not_allowed);
    return;
  }

  if (!GetPermissions().MayUserAccess(user, method)) {
    session->SendResponse(http::status::forbidden);
    return;
  }

  auto file_path = GetFilePathFromUri(uri);
  if (file_path.empty()) {
    LOG_ERR(session->GetLoggingPrefix() << ": Illegal request target");
    session->SendResponse(http::status::bad_request);
    return;
  }

  http::file_body::value_type body;
  boost::beast::error_code ec;
  body.open(file_path.string().c_str(), beast::file_mode::scan, ec);
  if (ec == errc::no_such_file_or_directory) {
    LOG_ERR("Requested non-existing file " << file_path);
    session->SendResponse(http::status::not_found);
    return;
  }

  if (ec) {
    LOG_ERR("Could not open requested file " << file_path << ": "
                                             << ec.message());
    session->SendResponse(http::status::internal_server_error, ec.message());
    return;
  }

  LOG_IFO("Sending requested file " << file_path);

  auto size = body.size();
  if (method == api::kHead) {
    HttpsSession::EmptyResponse resp;
    resp.result(http::status::ok);
    resp.set(http::field::content_type, GetMimeType(file_path));
    resp.content_length(size);
    session->SendResponse(std::move(resp));
  } else {
    HttpsSession::FileResponse resp{http::status::ok, 11, std::move(body)};
    resp.set(http::field::content_type, GetMimeType(file_path));
    resp.content_length(size);
    session->SendResponse(std::move(resp));
  }
}

void FileSystemRoute::ReadConfiguration(
    const nlohmann::json::const_iterator& route_it) {
  path_ = (*route_it)["path"].get<std::string>();
}

void FileSystemRoute::LogCreation() {
  LOG_IFO("Added filesystem route " << GetBaseUri() << " serving " << path_
                                    << (IsEnabled() ? "" : " (disabled)"));
}

fs::path FileSystemRoute::GetFilePathFromUri(const std::string& uri) const {
  YOGI_ASSERT(uri.find(GetBaseUri()) == 0);

  auto base_len = GetBaseUri().size() + (GetBaseUri().back() == '/' ? 1u : 0u);
  auto rel_path = fs::path(uri.substr(base_len));

  for (auto& dir : rel_path) {
    if (dir.filename_is_dot_dot()) {
      return {};
    }
  }

  auto file_path = fs::path(path_) / rel_path;
  boost::system::error_code ec;
  if (fs::is_directory(file_path, ec)) {
    file_path /= "index.html";
  }

  return file_path;
}

const std::string& FileSystemRoute::GetMimeType(
    const boost::filesystem::path& file) const {
  auto ext = file.extension().string();
  if (!ext.empty()) {
    YOGI_ASSERT(ext.front() == '.');
    ext = ext.substr(1);
  }

  return FileExtensionToMimeType(ext);
}

}  // namespace detail
}  // namespace web
}  // namespace objects
