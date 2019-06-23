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

#include <memory>
#include <utility>
#include <functional>

namespace utils {

template <typename Obj, typename... Args>
inline std::function<void(Args...)> BindWeak(void (Obj::*fn)(Args...),
                                             Obj* obj) {
  auto shared_obj = obj->shared_from_this();
  auto weak_obj =
      std::weak_ptr<typename decltype(shared_obj)::element_type>{shared_obj};

  std::function<void(Args...)> wrapper = [weak_obj, fn](Args&&... args) {
    auto shared_obj = std::static_pointer_cast<Obj>(weak_obj.lock());
    if (!shared_obj) return;

    ((*shared_obj).*fn)(std::forward<Args>(args)...);
  };

  return wrapper;
}

template <typename Obj, typename... Args, typename LastArg, typename BoundArg>
inline std::function<void(Args...)> BindWeak(void (Obj::*fn)(Args..., LastArg),
                                             Obj* obj, BoundArg&& bound_arg) {
  auto shared_obj = obj->shared_from_this();
  auto weak_obj =
      std::weak_ptr<typename decltype(shared_obj)::element_type>{shared_obj};

  std::function<void(Args...)> wrapper =
      [weak_obj, fn,
       bound_arg = std::forward<BoundArg>(bound_arg)](Args&&... args) {
        auto shared_obj = std::static_pointer_cast<Obj>(weak_obj.lock());
        if (!shared_obj) return;

        ((*shared_obj).*fn)(std::forward<Args>(args)..., bound_arg);
      };

  return wrapper;
}

}  // namespace utils
