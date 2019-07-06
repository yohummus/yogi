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

#include "../../../../api/enums.h"

#include <boost/beast/http/verb.hpp>

namespace objects {
namespace web {
namespace detail {
namespace {

boost::beast::http::verb MethodToVerb(api::RequestMethods method) {
  using boost::beast::http::verb;
  switch (method) {
    case api::kGet:
      return verb::get;

    case api::kHead:
      return verb::head;

    case api::kPost:
      return verb::post;

    case api::kPut:
      return verb::put;

    case api::kDelete:
      return verb::delete_;

    case api::kPatch:
      return verb::patch;

    default:
      YOGI_NEVER_REACHED;
      return verb::unknown;
  }
}

api::RequestMethods VerbToMethod(boost::beast::http::verb verb) {
  using boost::beast::http::verb;
  switch (verb) {
    case verb::get:
      return api::kGet;

    case verb::head:
      return api::kHead;

    case verb::post:
      return api::kPost;

    case verb::put:
      return api::kPut;

    case verb::delete_:
      return api::kDelete;

    case verb::patch:
      return api::kPatch;

    default:
      return api::RequestMethods::kNoMethod;
  }
}

}  // anonymous namespace
}  // namespace detail
}  // namespace web
}  // namespace objects
