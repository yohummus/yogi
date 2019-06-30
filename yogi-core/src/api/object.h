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
#include "errors.h"
#include "constants.h"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>

namespace api {

enum class ObjectType {
  kDummy,  // For testing
  kContext,
  kLogger,
  kTimer,
  kBranch,
  kConfiguration,
  kSignalSet,
  kWebServer,
  kWebRoute,
  kWebProcess,
};

typedef void* ObjectHandle;

class ExposedObject : public std::enable_shared_from_this<ExposedObject> {
 public:
  ExposedObject() = default;
  virtual ~ExposedObject() = default;

  virtual ObjectType Type() const = 0;

  const std::string& TypeName() const;
  std::string Format(std::string fmt = kDefaultObjectFormat) const;
  ObjectHandle Handle() { return static_cast<ObjectHandle>(this); }

  template <typename TO>
  std::shared_ptr<TO> Cast() {
    // Specialization for ExposeObject type is below this class
    if (Type() != TO::StaticType()) {
      throw Error(YOGI_ERR_WRONG_OBJECT_TYPE);
    }

    return std::static_pointer_cast<TO>(this->shared_from_this());
  }

 private:
  // noncopyable
  ExposedObject(const ExposedObject&) = delete;
  void operator=(const ExposedObject&) = delete;
};

template <>
inline std::shared_ptr<ExposedObject> ExposedObject::Cast() {
  return std::static_pointer_cast<ExposedObject>(this->shared_from_this());
}

typedef std::shared_ptr<ExposedObject> ObjectPtr;

template <typename TO, ObjectType TK>
class ExposedObjectT : public ExposedObject {
 public:
  static constexpr ObjectType StaticType() { return TK; }

  template <typename... TArgs>
  static std::shared_ptr<TO> Create(TArgs&&... args) {
    return std::make_shared<TO>(std::forward<TArgs>(args)...);
  }

  virtual ObjectType Type() const override { return StaticType(); };

  std::shared_ptr<TO> MakeSharedPtr() {
    return std::static_pointer_cast<TO>(this->shared_from_this());
  }

  std::weak_ptr<TO> MakeWeakPtr() { return {MakeSharedPtr()}; }
};

class ObjectRegister {
 public:
  template <typename TO = ExposedObject>
  static std::shared_ptr<TO> Get(ObjectHandle handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto obj = objects_.find(handle);

    if (obj == objects_.end()) {
      throw Error(YOGI_ERR_INVALID_HANDLE);
    }

    YOGI_ASSERT(obj->second->Handle() == handle);
    return obj->second->Cast<TO>();
  }

  template <typename TO, typename TP>
  static std::vector<std::shared_ptr<TO>> GetMatching(TP predicate) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::shared_ptr<TO>> vec;
    for (auto& obj : objects_) {
      if (obj.second->Type() == TO::StaticType()) {
        auto typed_obj = std::static_pointer_cast<TO>(obj.second);
        if (predicate(typed_obj)) {
          vec.emplace_back(std::move(typed_obj));
        }
      }
    }

    return vec;
  }

  template <typename TO>
  static std::vector<std::shared_ptr<TO>> GetAll() {
    return GetMatching<TO>([](auto&) { return true; });
  }

  static ObjectHandle Register(const ObjectPtr& obj);
  static void Destroy(ObjectHandle handle);
  static void DestroyAll();

 private:
  typedef std::unordered_map<ObjectHandle, ObjectPtr> ObjectsMap;

  static ObjectsMap TakeObjects();
  static bool DestroyUnusedObjects(ObjectsMap* objs);
  static void StopAllContexts(const ObjectsMap& objs);
  static void PollAllContexts(const ObjectsMap& objs);
  static void PrintObjectsStillInUse(const ObjectsMap& objs);

  static std::mutex mutex_;
  static ObjectsMap objects_;
};

}  // namespace api
