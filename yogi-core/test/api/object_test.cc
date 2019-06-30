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

#include "../common.h"
#include "../../src/api/object.h"

class Dummy : public api::ExposedObjectT<Dummy, api::ObjectType::kDummy> {};

class MyObject : public api::ExposedObjectT<MyObject, api::ObjectType::kTimer> {
 public:
  MyObject(int) {}
  ~MyObject() { ++dtor_calls_; }

  static int GetDtorCalls() { return dtor_calls_; }

 private:
  static int dtor_calls_;
};

int MyObject::dtor_calls_ = 0;

class ObjectTest : public TestFixture {
 protected:
  virtual void TearDown() override { api::ObjectRegister::DestroyAll(); }
};

TEST_F(ObjectTest, Create) {
  auto obj = MyObject::Create(123);
  EXPECT_TRUE(obj.unique());
}

TEST_F(ObjectTest, Type) {
  auto obj = MyObject::Create(123);
  EXPECT_EQ(obj->Type(), api::ObjectType::kTimer);
  EXPECT_EQ(MyObject::StaticType(), api::ObjectType::kTimer);
}

TEST_F(ObjectTest, Handle) {
  auto obj = MyObject::Create(123);
  EXPECT_NE(obj->Handle(), nullptr);
}

TEST_F(ObjectTest, Cast) {
  auto my_obj = MyObject::Create(123);
  auto obj = std::dynamic_pointer_cast<api::ExposedObject>(my_obj);
  auto my_obj_2 = obj->Cast<MyObject>();
  EXPECT_EQ(my_obj, my_obj_2);

  EXPECT_THROW(my_obj->Cast<Dummy>(), api::Error);
  EXPECT_THROW(obj->Cast<Dummy>(), api::Error);
}

TEST_F(ObjectTest, MakeWeakPtr) {
  auto obj = MyObject::Create(123);
  auto weak = obj->MakeWeakPtr();
  EXPECT_EQ(weak.lock(), obj);
}

TEST_F(ObjectTest, RegisterAndDestroyObject) {
  auto dtor_calls = MyObject::GetDtorCalls();

  auto obj = MyObject::Create(123);
  auto handle = api::ObjectRegister::Register(obj);
  EXPECT_FALSE(obj.unique());
  EXPECT_EQ(MyObject::GetDtorCalls(), dtor_calls);
  EXPECT_EQ(handle, obj->Handle());

  EXPECT_THROW(api::ObjectRegister::Destroy(nullptr), api::Error);
  EXPECT_THROW(api::ObjectRegister::Destroy(handle), api::Error);

  obj.reset();
  EXPECT_EQ(MyObject::GetDtorCalls(), dtor_calls);
  EXPECT_NO_THROW(api::ObjectRegister::Destroy(handle));
  EXPECT_EQ(MyObject::GetDtorCalls(), dtor_calls + 1);

  EXPECT_THROW(api::ObjectRegister::Destroy(handle), api::Error);
}

TEST_F(ObjectTest, GetRegisteredObject) {
  EXPECT_THROW(api::ObjectRegister::Get(nullptr), api::Error);

  auto obj = MyObject::Create(123);
  auto handle = api::ObjectRegister::Register(obj);

  api::ObjectPtr obj2 = api::ObjectRegister::Get(handle);
  EXPECT_EQ(obj2, obj);

  std::shared_ptr<MyObject> obj3 = api::ObjectRegister::Get<MyObject>(handle);
  EXPECT_EQ(obj3, obj);

  EXPECT_THROW(api::ObjectRegister::Get<Dummy>(handle), api::Error);
}

TEST_F(ObjectTest, GetAllRegisteredObjectsOfType) {
  auto obj1 = MyObject::Create(123);
  api::ObjectRegister::Register(obj1);
  auto obj2 = MyObject::Create(456);
  api::ObjectRegister::Register(obj2);
  auto dummy = Dummy::Create();
  api::ObjectRegister::Register(dummy);

  auto vec = api::ObjectRegister::GetAll<MyObject>();
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(std::count(vec.begin(), vec.end(), obj1), 1);
  EXPECT_EQ(std::count(vec.begin(), vec.end(), obj2), 1);
}

TEST_F(ObjectTest, DestroyAllObjects) {
  EXPECT_NO_THROW(api::ObjectRegister::DestroyAll());

  auto obj1 = MyObject::Create(123);
  auto obj2 = MyObject::Create(456);

  auto dtor_calls = MyObject::GetDtorCalls();
  api::ObjectRegister::Register(obj2);
  api::ObjectRegister::Register(obj1);
  EXPECT_EQ(MyObject::GetDtorCalls(), dtor_calls);

  obj1.reset();
  obj2.reset();
  api::ObjectRegister::DestroyAll();
  EXPECT_EQ(MyObject::GetDtorCalls(), dtor_calls + 2);
}
