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
#include "../api/errors.h"
#include "web_server.h"

#include <string>
#include <functional>

namespace objects {

class WebProcess
    : public api::ExposedObjectT<WebProcess, api::ObjectType::kWebProcess> {
 public:
  typedef std::function<void(/* TODO */)> ActionHandler;

  WebProcess(WebServerPtr server, std::string name, ActionHandler fn);

 private:
  const WebServerPtr server_;
};

typedef std::shared_ptr<WebProcess> WebProcessPtr;

}  // namespace objects
