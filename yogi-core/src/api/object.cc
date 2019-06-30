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
#include "../objects/context.h"

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
  std::vector<ObjectPtr> objs = TakeObjects();
  StopAllContexts(objs);

  while (RemoveUnusedObjects(&objs) || PollAllContexts(objs))
    ;

  if (!objs.empty()) {
    YOGI_DEBUG_ONLY(PrintObjectsStillInUse(objs));
    throw Error(YOGI_ERR_OBJECT_STILL_USED);
  }
}

ObjectRegister::ObjectsVector ObjectRegister::TakeObjects() {
  ObjectsVector objs;

  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& entry : objects_) {
    objs.push_back(std::move(entry.second));
  }
  objects_.clear();

  return objs;
}

bool ObjectRegister::RemoveUnusedObjects(ObjectsVector* objs) {
  bool destroyed_some = false;

  auto it = objs->begin();
  while (it != objs->end()) {
    if (it->unique()) {
      it = objs->erase(it);
      destroyed_some = true;
    } else {
      ++it;
    }
  }

  return destroyed_some;
}

void ObjectRegister::StopAllContexts(const ObjectsVector& objs) {
  for (auto& context : GetAll<objects::Context>(objs)) {
    context->Stop();
    bool stopped = context->WaitForStopped(std::chrono::nanoseconds::max());
    YOGI_ASSERT(stopped);
  }
}

bool ObjectRegister::PollAllContexts(const ObjectsVector& objs) {
  bool polled_some = false;
  for (auto& context : GetAll<objects::Context>(objs)) {
    int n = context->Poll();
    if (n) polled_some = true;
  }

  return polled_some;
}

void ObjectRegister::PrintObjectsStillInUse(const ObjectsVector& objs) {
  std::cout << "Objects still in use after DestroyAll():" << std::endl;
  for (auto& obj : objs) {
    std::cout << obj->Format() << std::endl;
  }
  std::cout << std::endl;
}

}  // namespace api
