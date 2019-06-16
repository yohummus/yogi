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

#include "../../config.h"
#include "../../api/errors.h"
#include "web_server.h"

#include <string>
#include <functional>

namespace objects {
namespace web {

class WebRoute
    : public api::ExposedObjectT<WebRoute, api::ObjectType::kWebRoute> {
 public:
  typedef std::function<void(/* TODO */)> RequestHandler;

  WebRoute(WebServerPtr server, std::string base_uri, RequestHandler fn);

 private:
  const WebServerPtr server_;
};

typedef std::shared_ptr<WebRoute> WebRoutePtr;

}  // namespace web
}  // namespace objects
