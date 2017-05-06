// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: yogi_00000c07.proto

#ifndef PROTOBUF_yogi_5f00000c07_2eproto__INCLUDED
#define PROTOBUF_yogi_5f00000c07_2eproto__INCLUDED

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

namespace yogi_00000c07_ns {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_yogi_5f00000c07_2eproto();
void protobuf_AssignDesc_yogi_5f00000c07_2eproto();
void protobuf_ShutdownFile_yogi_5f00000c07_2eproto();

class GatherMessage;
class MasterMessage;
class PublishMessage;
class ScatterMessage;
class SlaveMessage;

// ===================================================================

class ScatterMessage : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:yogi_00000c07_ns.ScatterMessage) */ {
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
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
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

  // @@protoc_insertion_point(class_scope:yogi_00000c07_ns.ScatterMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_yogi_5f00000c07_2eproto();
  friend void protobuf_AssignDesc_yogi_5f00000c07_2eproto();
  friend void protobuf_ShutdownFile_yogi_5f00000c07_2eproto();

  void InitAsDefaultInstance();
  static ScatterMessage* default_instance_;
};
// -------------------------------------------------------------------

class GatherMessage : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:yogi_00000c07_ns.GatherMessage) */ {
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
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
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

  // accessors -------------------------------------------------------

  // optional uint64 timestamp = 1;
  void clear_timestamp();
  static const int kTimestampFieldNumber = 1;
  ::google::protobuf::uint64 timestamp() const;
  void set_timestamp(::google::protobuf::uint64 value);

  // repeated sint32 value = 2 [packed = true];
  int value_size() const;
  void clear_value();
  static const int kValueFieldNumber = 2;
  ::google::protobuf::int32 value(int index) const;
  void set_value(int index, ::google::protobuf::int32 value);
  void add_value(::google::protobuf::int32 value);
  const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
      value() const;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
      mutable_value();

  // @@protoc_insertion_point(class_scope:yogi_00000c07_ns.GatherMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::uint64 timestamp_;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 > value_;
  mutable int _value_cached_byte_size_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_yogi_5f00000c07_2eproto();
  friend void protobuf_AssignDesc_yogi_5f00000c07_2eproto();
  friend void protobuf_ShutdownFile_yogi_5f00000c07_2eproto();

  void InitAsDefaultInstance();
  static GatherMessage* default_instance_;
};
// -------------------------------------------------------------------

class PublishMessage : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:yogi_00000c07_ns.PublishMessage) */ {
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
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
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

  // accessors -------------------------------------------------------

  // optional uint64 timestamp = 1;
  void clear_timestamp();
  static const int kTimestampFieldNumber = 1;
  ::google::protobuf::uint64 timestamp() const;
  void set_timestamp(::google::protobuf::uint64 value);

  // repeated sint32 value = 2 [packed = true];
  int value_size() const;
  void clear_value();
  static const int kValueFieldNumber = 2;
  ::google::protobuf::int32 value(int index) const;
  void set_value(int index, ::google::protobuf::int32 value);
  void add_value(::google::protobuf::int32 value);
  const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
      value() const;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
      mutable_value();

  // @@protoc_insertion_point(class_scope:yogi_00000c07_ns.PublishMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::uint64 timestamp_;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 > value_;
  mutable int _value_cached_byte_size_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_yogi_5f00000c07_2eproto();
  friend void protobuf_AssignDesc_yogi_5f00000c07_2eproto();
  friend void protobuf_ShutdownFile_yogi_5f00000c07_2eproto();

  void InitAsDefaultInstance();
  static PublishMessage* default_instance_;
};
// -------------------------------------------------------------------

class MasterMessage : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:yogi_00000c07_ns.MasterMessage) */ {
 public:
  MasterMessage();
  virtual ~MasterMessage();

  MasterMessage(const MasterMessage& from);

  inline MasterMessage& operator=(const MasterMessage& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const MasterMessage& default_instance();

  void Swap(MasterMessage* other);

  // implements Message ----------------------------------------------

  inline MasterMessage* New() const { return New(NULL); }

  MasterMessage* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const MasterMessage& from);
  void MergeFrom(const MasterMessage& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(MasterMessage* other);
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

  // @@protoc_insertion_point(class_scope:yogi_00000c07_ns.MasterMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_yogi_5f00000c07_2eproto();
  friend void protobuf_AssignDesc_yogi_5f00000c07_2eproto();
  friend void protobuf_ShutdownFile_yogi_5f00000c07_2eproto();

  void InitAsDefaultInstance();
  static MasterMessage* default_instance_;
};
// -------------------------------------------------------------------

class SlaveMessage : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:yogi_00000c07_ns.SlaveMessage) */ {
 public:
  SlaveMessage();
  virtual ~SlaveMessage();

  SlaveMessage(const SlaveMessage& from);

  inline SlaveMessage& operator=(const SlaveMessage& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const SlaveMessage& default_instance();

  void Swap(SlaveMessage* other);

  // implements Message ----------------------------------------------

  inline SlaveMessage* New() const { return New(NULL); }

  SlaveMessage* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const SlaveMessage& from);
  void MergeFrom(const SlaveMessage& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(SlaveMessage* other);
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

  // optional uint64 timestamp = 1;
  void clear_timestamp();
  static const int kTimestampFieldNumber = 1;
  ::google::protobuf::uint64 timestamp() const;
  void set_timestamp(::google::protobuf::uint64 value);

  // repeated sint32 value = 2 [packed = true];
  int value_size() const;
  void clear_value();
  static const int kValueFieldNumber = 2;
  ::google::protobuf::int32 value(int index) const;
  void set_value(int index, ::google::protobuf::int32 value);
  void add_value(::google::protobuf::int32 value);
  const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
      value() const;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
      mutable_value();

  // @@protoc_insertion_point(class_scope:yogi_00000c07_ns.SlaveMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::uint64 timestamp_;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 > value_;
  mutable int _value_cached_byte_size_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_yogi_5f00000c07_2eproto();
  friend void protobuf_AssignDesc_yogi_5f00000c07_2eproto();
  friend void protobuf_ShutdownFile_yogi_5f00000c07_2eproto();

  void InitAsDefaultInstance();
  static SlaveMessage* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// ScatterMessage

// -------------------------------------------------------------------

// GatherMessage

// optional uint64 timestamp = 1;
inline void GatherMessage::clear_timestamp() {
  timestamp_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 GatherMessage::timestamp() const {
  // @@protoc_insertion_point(field_get:yogi_00000c07_ns.GatherMessage.timestamp)
  return timestamp_;
}
inline void GatherMessage::set_timestamp(::google::protobuf::uint64 value) {
  
  timestamp_ = value;
  // @@protoc_insertion_point(field_set:yogi_00000c07_ns.GatherMessage.timestamp)
}

// repeated sint32 value = 2 [packed = true];
inline int GatherMessage::value_size() const {
  return value_.size();
}
inline void GatherMessage::clear_value() {
  value_.Clear();
}
inline ::google::protobuf::int32 GatherMessage::value(int index) const {
  // @@protoc_insertion_point(field_get:yogi_00000c07_ns.GatherMessage.value)
  return value_.Get(index);
}
inline void GatherMessage::set_value(int index, ::google::protobuf::int32 value) {
  value_.Set(index, value);
  // @@protoc_insertion_point(field_set:yogi_00000c07_ns.GatherMessage.value)
}
inline void GatherMessage::add_value(::google::protobuf::int32 value) {
  value_.Add(value);
  // @@protoc_insertion_point(field_add:yogi_00000c07_ns.GatherMessage.value)
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
GatherMessage::value() const {
  // @@protoc_insertion_point(field_list:yogi_00000c07_ns.GatherMessage.value)
  return value_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
GatherMessage::mutable_value() {
  // @@protoc_insertion_point(field_mutable_list:yogi_00000c07_ns.GatherMessage.value)
  return &value_;
}

// -------------------------------------------------------------------

// PublishMessage

// optional uint64 timestamp = 1;
inline void PublishMessage::clear_timestamp() {
  timestamp_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 PublishMessage::timestamp() const {
  // @@protoc_insertion_point(field_get:yogi_00000c07_ns.PublishMessage.timestamp)
  return timestamp_;
}
inline void PublishMessage::set_timestamp(::google::protobuf::uint64 value) {
  
  timestamp_ = value;
  // @@protoc_insertion_point(field_set:yogi_00000c07_ns.PublishMessage.timestamp)
}

// repeated sint32 value = 2 [packed = true];
inline int PublishMessage::value_size() const {
  return value_.size();
}
inline void PublishMessage::clear_value() {
  value_.Clear();
}
inline ::google::protobuf::int32 PublishMessage::value(int index) const {
  // @@protoc_insertion_point(field_get:yogi_00000c07_ns.PublishMessage.value)
  return value_.Get(index);
}
inline void PublishMessage::set_value(int index, ::google::protobuf::int32 value) {
  value_.Set(index, value);
  // @@protoc_insertion_point(field_set:yogi_00000c07_ns.PublishMessage.value)
}
inline void PublishMessage::add_value(::google::protobuf::int32 value) {
  value_.Add(value);
  // @@protoc_insertion_point(field_add:yogi_00000c07_ns.PublishMessage.value)
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
PublishMessage::value() const {
  // @@protoc_insertion_point(field_list:yogi_00000c07_ns.PublishMessage.value)
  return value_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
PublishMessage::mutable_value() {
  // @@protoc_insertion_point(field_mutable_list:yogi_00000c07_ns.PublishMessage.value)
  return &value_;
}

// -------------------------------------------------------------------

// MasterMessage

// -------------------------------------------------------------------

// SlaveMessage

// optional uint64 timestamp = 1;
inline void SlaveMessage::clear_timestamp() {
  timestamp_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 SlaveMessage::timestamp() const {
  // @@protoc_insertion_point(field_get:yogi_00000c07_ns.SlaveMessage.timestamp)
  return timestamp_;
}
inline void SlaveMessage::set_timestamp(::google::protobuf::uint64 value) {
  
  timestamp_ = value;
  // @@protoc_insertion_point(field_set:yogi_00000c07_ns.SlaveMessage.timestamp)
}

// repeated sint32 value = 2 [packed = true];
inline int SlaveMessage::value_size() const {
  return value_.size();
}
inline void SlaveMessage::clear_value() {
  value_.Clear();
}
inline ::google::protobuf::int32 SlaveMessage::value(int index) const {
  // @@protoc_insertion_point(field_get:yogi_00000c07_ns.SlaveMessage.value)
  return value_.Get(index);
}
inline void SlaveMessage::set_value(int index, ::google::protobuf::int32 value) {
  value_.Set(index, value);
  // @@protoc_insertion_point(field_set:yogi_00000c07_ns.SlaveMessage.value)
}
inline void SlaveMessage::add_value(::google::protobuf::int32 value) {
  value_.Add(value);
  // @@protoc_insertion_point(field_add:yogi_00000c07_ns.SlaveMessage.value)
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
SlaveMessage::value() const {
  // @@protoc_insertion_point(field_list:yogi_00000c07_ns.SlaveMessage.value)
  return value_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
SlaveMessage::mutable_value() {
  // @@protoc_insertion_point(field_mutable_list:yogi_00000c07_ns.SlaveMessage.value)
  return &value_;
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace yogi_00000c07_ns

struct yogi_00000c07
{
    typedef yogi_00000c07_ns::PublishMessage PublishMessage;
    typedef yogi_00000c07_ns::ScatterMessage ScatterMessage;
    typedef yogi_00000c07_ns::GatherMessage  GatherMessage;
    typedef yogi_00000c07_ns::MasterMessage  MasterMessage;
    typedef yogi_00000c07_ns::SlaveMessage   SlaveMessage;

    enum {
        SIGNATURE = 0x00000c07
    };
};

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_yogi_5f00000c07_2eproto__INCLUDED