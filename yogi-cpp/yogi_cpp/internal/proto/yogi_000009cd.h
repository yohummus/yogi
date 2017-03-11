// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: cppchirp_000009cd.proto

#ifndef PROTOBUF_cppchirp_5f000009cd_2eproto__INCLUDED
#define PROTOBUF_cppchirp_5f000009cd_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace yogi {
namespace internal {
namespace yogi_000009cd_ns {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_cppchirp_5f000009cd_2eproto();
void protobuf_AssignDesc_cppchirp_5f000009cd_2eproto();
void protobuf_ShutdownFile_cppchirp_5f000009cd_2eproto();

class GatherMessage;
class GatherMessage_Pair;
class PublishMessage;
class PublishMessage_Pair;
class ScatterMessage;

// ===================================================================

class ScatterMessage : public ::google::protobuf::Message {
 public:
  ScatterMessage();
  virtual ~ScatterMessage();

  ScatterMessage(const ScatterMessage& from);

  inline ScatterMessage& operator=(const ScatterMessage& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ScatterMessage& default_instance();

  void Swap(ScatterMessage* other);

  // implements Message ----------------------------------------------

  inline ScatterMessage* New() const { return New(NULL); }

  ScatterMessage* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ScatterMessage& from);
  void MergeFrom(const ScatterMessage& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ScatterMessage* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:cppchirp.internal.cppchirp_000009cd.ScatterMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_cppchirp_5f000009cd_2eproto();
  friend void protobuf_AssignDesc_cppchirp_5f000009cd_2eproto();
  friend void protobuf_ShutdownFile_cppchirp_5f000009cd_2eproto();

  void InitAsDefaultInstance();
  static ScatterMessage* default_instance_;
};
// -------------------------------------------------------------------

class GatherMessage_Pair : public ::google::protobuf::Message {
 public:
  GatherMessage_Pair();
  virtual ~GatherMessage_Pair();

  GatherMessage_Pair(const GatherMessage_Pair& from);

  inline GatherMessage_Pair& operator=(const GatherMessage_Pair& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const GatherMessage_Pair& default_instance();

  void Swap(GatherMessage_Pair* other);

  // implements Message ----------------------------------------------

  inline GatherMessage_Pair* New() const { return New(NULL); }

  GatherMessage_Pair* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const GatherMessage_Pair& from);
  void MergeFrom(const GatherMessage_Pair& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(GatherMessage_Pair* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string first = 1;
  void clear_first();
  static const int kFirstFieldNumber = 1;
  const ::std::string& first() const;
  void set_first(const ::std::string& value);
  void set_first(const char* value);
  void set_first(const char* value, size_t size);
  ::std::string* mutable_first();
  ::std::string* release_first();
  void set_allocated_first(::std::string* first);

  // optional string second = 2;
  void clear_second();
  static const int kSecondFieldNumber = 2;
  const ::std::string& second() const;
  void set_second(const ::std::string& value);
  void set_second(const char* value);
  void set_second(const char* value, size_t size);
  ::std::string* mutable_second();
  ::std::string* release_second();
  void set_allocated_second(::std::string* second);

  // @@protoc_insertion_point(class_scope:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr first_;
  ::google::protobuf::internal::ArenaStringPtr second_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_cppchirp_5f000009cd_2eproto();
  friend void protobuf_AssignDesc_cppchirp_5f000009cd_2eproto();
  friend void protobuf_ShutdownFile_cppchirp_5f000009cd_2eproto();

  void InitAsDefaultInstance();
  static GatherMessage_Pair* default_instance_;
};
// -------------------------------------------------------------------

class GatherMessage : public ::google::protobuf::Message {
 public:
  GatherMessage();
  virtual ~GatherMessage();

  GatherMessage(const GatherMessage& from);

  inline GatherMessage& operator=(const GatherMessage& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const GatherMessage& default_instance();

  void Swap(GatherMessage* other);

  // implements Message ----------------------------------------------

  inline GatherMessage* New() const { return New(NULL); }

  GatherMessage* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const GatherMessage& from);
  void MergeFrom(const GatherMessage& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(GatherMessage* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  typedef GatherMessage_Pair Pair;

  // accessors -------------------------------------------------------

  // optional uint64 timestamp = 1;
  void clear_timestamp();
  static const int kTimestampFieldNumber = 1;
  ::google::protobuf::uint64 timestamp() const;
  void set_timestamp(::google::protobuf::uint64 value);

  // optional .cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair value = 2;
  bool has_value() const;
  void clear_value();
  static const int kValueFieldNumber = 2;
  const ::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair& value() const;
  ::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair* mutable_value();
  ::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair* release_value();
  void set_allocated_value(::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair* value);

  // @@protoc_insertion_point(class_scope:cppchirp.internal.cppchirp_000009cd.GatherMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::uint64 timestamp_;
  ::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair* value_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_cppchirp_5f000009cd_2eproto();
  friend void protobuf_AssignDesc_cppchirp_5f000009cd_2eproto();
  friend void protobuf_ShutdownFile_cppchirp_5f000009cd_2eproto();

  void InitAsDefaultInstance();
  static GatherMessage* default_instance_;
};
// -------------------------------------------------------------------

class PublishMessage_Pair : public ::google::protobuf::Message {
 public:
  PublishMessage_Pair();
  virtual ~PublishMessage_Pair();

  PublishMessage_Pair(const PublishMessage_Pair& from);

  inline PublishMessage_Pair& operator=(const PublishMessage_Pair& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const PublishMessage_Pair& default_instance();

  void Swap(PublishMessage_Pair* other);

  // implements Message ----------------------------------------------

  inline PublishMessage_Pair* New() const { return New(NULL); }

  PublishMessage_Pair* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const PublishMessage_Pair& from);
  void MergeFrom(const PublishMessage_Pair& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(PublishMessage_Pair* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string first = 1;
  void clear_first();
  static const int kFirstFieldNumber = 1;
  const ::std::string& first() const;
  void set_first(const ::std::string& value);
  void set_first(const char* value);
  void set_first(const char* value, size_t size);
  ::std::string* mutable_first();
  ::std::string* release_first();
  void set_allocated_first(::std::string* first);

  // optional string second = 2;
  void clear_second();
  static const int kSecondFieldNumber = 2;
  const ::std::string& second() const;
  void set_second(const ::std::string& value);
  void set_second(const char* value);
  void set_second(const char* value, size_t size);
  ::std::string* mutable_second();
  ::std::string* release_second();
  void set_allocated_second(::std::string* second);

  // @@protoc_insertion_point(class_scope:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr first_;
  ::google::protobuf::internal::ArenaStringPtr second_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_cppchirp_5f000009cd_2eproto();
  friend void protobuf_AssignDesc_cppchirp_5f000009cd_2eproto();
  friend void protobuf_ShutdownFile_cppchirp_5f000009cd_2eproto();

  void InitAsDefaultInstance();
  static PublishMessage_Pair* default_instance_;
};
// -------------------------------------------------------------------

class PublishMessage : public ::google::protobuf::Message {
 public:
  PublishMessage();
  virtual ~PublishMessage();

  PublishMessage(const PublishMessage& from);

  inline PublishMessage& operator=(const PublishMessage& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const PublishMessage& default_instance();

  void Swap(PublishMessage* other);

  // implements Message ----------------------------------------------

  inline PublishMessage* New() const { return New(NULL); }

  PublishMessage* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const PublishMessage& from);
  void MergeFrom(const PublishMessage& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(PublishMessage* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  typedef PublishMessage_Pair Pair;

  // accessors -------------------------------------------------------

  // optional uint64 timestamp = 1;
  void clear_timestamp();
  static const int kTimestampFieldNumber = 1;
  ::google::protobuf::uint64 timestamp() const;
  void set_timestamp(::google::protobuf::uint64 value);

  // optional .cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair value = 2;
  bool has_value() const;
  void clear_value();
  static const int kValueFieldNumber = 2;
  const ::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair& value() const;
  ::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair* mutable_value();
  ::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair* release_value();
  void set_allocated_value(::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair* value);

  // @@protoc_insertion_point(class_scope:cppchirp.internal.cppchirp_000009cd.PublishMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::uint64 timestamp_;
  ::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair* value_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_cppchirp_5f000009cd_2eproto();
  friend void protobuf_AssignDesc_cppchirp_5f000009cd_2eproto();
  friend void protobuf_ShutdownFile_cppchirp_5f000009cd_2eproto();

  void InitAsDefaultInstance();
  static PublishMessage* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// ScatterMessage

// -------------------------------------------------------------------

// GatherMessage_Pair

// optional string first = 1;
inline void GatherMessage_Pair::clear_first() {
  first_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& GatherMessage_Pair::first() const {
  // @@protoc_insertion_point(field_get:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.first)
  return first_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void GatherMessage_Pair::set_first(const ::std::string& value) {

  first_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.first)
}
inline void GatherMessage_Pair::set_first(const char* value) {

  first_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.first)
}
inline void GatherMessage_Pair::set_first(const char* value, size_t size) {

  first_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.first)
}
inline ::std::string* GatherMessage_Pair::mutable_first() {

  // @@protoc_insertion_point(field_mutable:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.first)
  return first_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* GatherMessage_Pair::release_first() {
  // @@protoc_insertion_point(field_release:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.first)

  return first_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void GatherMessage_Pair::set_allocated_first(::std::string* first) {
  if (first != NULL) {

  } else {

  }
  first_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), first);
  // @@protoc_insertion_point(field_set_allocated:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.first)
}

// optional string second = 2;
inline void GatherMessage_Pair::clear_second() {
  second_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& GatherMessage_Pair::second() const {
  // @@protoc_insertion_point(field_get:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.second)
  return second_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void GatherMessage_Pair::set_second(const ::std::string& value) {

  second_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.second)
}
inline void GatherMessage_Pair::set_second(const char* value) {

  second_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.second)
}
inline void GatherMessage_Pair::set_second(const char* value, size_t size) {

  second_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.second)
}
inline ::std::string* GatherMessage_Pair::mutable_second() {

  // @@protoc_insertion_point(field_mutable:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.second)
  return second_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* GatherMessage_Pair::release_second() {
  // @@protoc_insertion_point(field_release:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.second)

  return second_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void GatherMessage_Pair::set_allocated_second(::std::string* second) {
  if (second != NULL) {

  } else {

  }
  second_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), second);
  // @@protoc_insertion_point(field_set_allocated:cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair.second)
}

// -------------------------------------------------------------------

// GatherMessage

// optional uint64 timestamp = 1;
inline void GatherMessage::clear_timestamp() {
  timestamp_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 GatherMessage::timestamp() const {
  // @@protoc_insertion_point(field_get:cppchirp.internal.cppchirp_000009cd.GatherMessage.timestamp)
  return timestamp_;
}
inline void GatherMessage::set_timestamp(::google::protobuf::uint64 value) {

  timestamp_ = value;
  // @@protoc_insertion_point(field_set:cppchirp.internal.cppchirp_000009cd.GatherMessage.timestamp)
}

// optional .cppchirp.internal.cppchirp_000009cd.GatherMessage.Pair value = 2;
inline bool GatherMessage::has_value() const {
  return !_is_default_instance_ && value_ != NULL;
}
inline void GatherMessage::clear_value() {
  if (GetArenaNoVirtual() == NULL && value_ != NULL) delete value_;
  value_ = NULL;
}
inline const ::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair& GatherMessage::value() const {
  // @@protoc_insertion_point(field_get:cppchirp.internal.cppchirp_000009cd.GatherMessage.value)
  return value_ != NULL ? *value_ : *default_instance_->value_;
}
inline ::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair* GatherMessage::mutable_value() {

  if (value_ == NULL) {
    value_ = new ::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair;
  }
  // @@protoc_insertion_point(field_mutable:cppchirp.internal.cppchirp_000009cd.GatherMessage.value)
  return value_;
}
inline ::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair* GatherMessage::release_value() {
  // @@protoc_insertion_point(field_release:cppchirp.internal.cppchirp_000009cd.GatherMessage.value)

  ::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair* temp = value_;
  value_ = NULL;
  return temp;
}
inline void GatherMessage::set_allocated_value(::yogi::internal::yogi_000009cd_ns::GatherMessage_Pair* value) {
  delete value_;
  value_ = value;
  if (value) {

  } else {

  }
  // @@protoc_insertion_point(field_set_allocated:cppchirp.internal.cppchirp_000009cd.GatherMessage.value)
}

// -------------------------------------------------------------------

// PublishMessage_Pair

// optional string first = 1;
inline void PublishMessage_Pair::clear_first() {
  first_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& PublishMessage_Pair::first() const {
  // @@protoc_insertion_point(field_get:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.first)
  return first_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void PublishMessage_Pair::set_first(const ::std::string& value) {

  first_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.first)
}
inline void PublishMessage_Pair::set_first(const char* value) {

  first_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.first)
}
inline void PublishMessage_Pair::set_first(const char* value, size_t size) {

  first_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.first)
}
inline ::std::string* PublishMessage_Pair::mutable_first() {

  // @@protoc_insertion_point(field_mutable:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.first)
  return first_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* PublishMessage_Pair::release_first() {
  // @@protoc_insertion_point(field_release:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.first)

  return first_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void PublishMessage_Pair::set_allocated_first(::std::string* first) {
  if (first != NULL) {

  } else {

  }
  first_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), first);
  // @@protoc_insertion_point(field_set_allocated:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.first)
}

// optional string second = 2;
inline void PublishMessage_Pair::clear_second() {
  second_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& PublishMessage_Pair::second() const {
  // @@protoc_insertion_point(field_get:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.second)
  return second_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void PublishMessage_Pair::set_second(const ::std::string& value) {

  second_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.second)
}
inline void PublishMessage_Pair::set_second(const char* value) {

  second_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.second)
}
inline void PublishMessage_Pair::set_second(const char* value, size_t size) {

  second_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.second)
}
inline ::std::string* PublishMessage_Pair::mutable_second() {

  // @@protoc_insertion_point(field_mutable:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.second)
  return second_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* PublishMessage_Pair::release_second() {
  // @@protoc_insertion_point(field_release:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.second)

  return second_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void PublishMessage_Pair::set_allocated_second(::std::string* second) {
  if (second != NULL) {

  } else {

  }
  second_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), second);
  // @@protoc_insertion_point(field_set_allocated:cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair.second)
}

// -------------------------------------------------------------------

// PublishMessage

// optional uint64 timestamp = 1;
inline void PublishMessage::clear_timestamp() {
  timestamp_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 PublishMessage::timestamp() const {
  // @@protoc_insertion_point(field_get:cppchirp.internal.cppchirp_000009cd.PublishMessage.timestamp)
  return timestamp_;
}
inline void PublishMessage::set_timestamp(::google::protobuf::uint64 value) {

  timestamp_ = value;
  // @@protoc_insertion_point(field_set:cppchirp.internal.cppchirp_000009cd.PublishMessage.timestamp)
}

// optional .cppchirp.internal.cppchirp_000009cd.PublishMessage.Pair value = 2;
inline bool PublishMessage::has_value() const {
  return !_is_default_instance_ && value_ != NULL;
}
inline void PublishMessage::clear_value() {
  if (GetArenaNoVirtual() == NULL && value_ != NULL) delete value_;
  value_ = NULL;
}
inline const ::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair& PublishMessage::value() const {
  // @@protoc_insertion_point(field_get:cppchirp.internal.cppchirp_000009cd.PublishMessage.value)
  return value_ != NULL ? *value_ : *default_instance_->value_;
}
inline ::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair* PublishMessage::mutable_value() {

  if (value_ == NULL) {
    value_ = new ::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair;
  }
  // @@protoc_insertion_point(field_mutable:cppchirp.internal.cppchirp_000009cd.PublishMessage.value)
  return value_;
}
inline ::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair* PublishMessage::release_value() {
  // @@protoc_insertion_point(field_release:cppchirp.internal.cppchirp_000009cd.PublishMessage.value)

  ::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair* temp = value_;
  value_ = NULL;
  return temp;
}
inline void PublishMessage::set_allocated_value(::yogi::internal::yogi_000009cd_ns::PublishMessage_Pair* value) {
  delete value_;
  value_ = value;
  if (value) {

  } else {

  }
  // @@protoc_insertion_point(field_set_allocated:cppchirp.internal.cppchirp_000009cd.PublishMessage.value)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace yogi_000009cd_ns

struct yogi_000009cd
{
    typedef yogi_000009cd_ns::PublishMessage PublishMessage;
    typedef yogi_000009cd_ns::ScatterMessage ScatterMessage;
    typedef yogi_000009cd_ns::GatherMessage  GatherMessage;

    enum {
        SIGNATURE = 0x000009cd
    };
};

}  // namespace internal
}  // namespace yogi

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_cppchirp_5f000009cd_2eproto__INCLUDED
