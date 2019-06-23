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

#include "object.h"

#include <boost/algorithm/string.hpp>
#include <limits>
#include <iostream>

namespace api {

const std::string& ExposedObject::TypeName() const {
  switch (Type()) {
    case ObjectType::kDummy: {
      static const std::string s = "Dummy";
      return s;
    }

    case ObjectType::kContext: {
      static const std::string s = "Context";
      return s;
    }

    case ObjectType::kLogger: {
      static const std::string s = "Logger";
      return s;
    }

    case ObjectType::kTimer: {
      static const std::string s = "Timer";
      return s;
    }

    case ObjectType::kBranch: {
      static const std::string s = "Branch";
      return s;
    }

    case ObjectType::kConfiguration: {
      static const std::string s = "Configuration";
      return s;
    }

    case ObjectType::kSignalSet: {
      static const std::string s = "SignalSet";
      return s;
    }

    case ObjectType::kWebServer: {
      static const std::string s = "WebServer";
      return s;
    }

    case ObjectType::kWebRoute: {
      static const std::string s = "WebRoute";
      return s;
    }

    case ObjectType::kWebProcess: {
      static const std::string s = "WebProcess";
      return s;
    }

    default: {
      YOGI_NEVER_REACHED;
      static const std::string s;
      return s;
    }
  }
}

std::string ExposedObject::Format(std::string fmt) const {
  boost::replace_all(fmt, "$T", TypeName());

  char buf[24];
  sprintf(buf, "%llx", reinterpret_cast<unsigned long long>(this));
  boost::replace_all(fmt, "$x", buf);
  sprintf(buf, "%llX", reinterpret_cast<unsigned long long>(this));
  boost::replace_all(fmt, "$X", buf);

  return fmt;
}

std::mutex ObjectRegister::mutex_;
// NOLINTNEXTLINE(cert-err58-cpp)
ObjectRegister::ObjectsMap ObjectRegister::objects_;

ObjectHandle ObjectRegister::Register(const ObjectPtr& obj) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto handle = obj->Handle();
  YOGI_ASSERT(!objects_.count(handle));
  objects_[handle] = obj;

  return handle;
}

void ObjectRegister::Destroy(ObjectHandle handle) {
  ObjectPtr obj;  // Hold it so it gets destroyed AFTER the lock is released

  std::lock_guard<std::mutex> lock(mutex_);
  auto it = objects_.find(handle);

  if (it == objects_.end()) {
    throw Error(YOGI_ERR_INVALID_HANDLE);
  }

  if (!it->second.unique()) {
    throw Error(YOGI_ERR_OBJECT_STILL_USED);
  }

  obj = it->second;
  objects_.erase(it);
}

void ObjectRegister::DestroyAll() {
  ObjectsMap objs;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    std::swap(objects_, objs);
  }

  auto old_size = std::numeric_limits<size_t>::max();
  while (objs.size() < old_size) {
    old_size = objs.size();

    for (auto it = objs.begin(); it != objs.end();) {
      if (it->second.unique()) {
        it = objs.erase(it);
      } else {
        ++it;
      }
    }
  }

  if (!objs.empty()) {
#ifndef NDEBUG
    std::cout << "Objects still in use after DestroyAll():" << std::endl;
    for (auto& obj : objs) {
      std::cout << obj.second->Format() << std::endl;
    }
    std::cout << std::endl;
#endif

    throw Error(YOGI_ERR_OBJECT_STILL_USED);
  }
}

}  // namespace api
