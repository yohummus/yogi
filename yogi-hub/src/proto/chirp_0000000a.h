// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: chirp_0000000a.proto

#ifndef PROTOBUF_chirp_5f0000000a_2eproto__INCLUDED
#define PROTOBUF_chirp_5f0000000a_2eproto__INCLUDED

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

namespace chirp_0000000a_ns {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_chirp_5f0000000a_2eproto();
void protobuf_AssignDesc_chirp_5f0000000a_2eproto();
void protobuf_ShutdownFile_chirp_5f0000000a_2eproto();

class GatherMessage;
class PublishMessage;
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

  // @@protoc_insertion_point(class_scope:chirp_0000000a_ns.ScatterMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_chirp_5f0000000a_2eproto();
  friend void protobuf_AssignDesc_chirp_5f0000000a_2eproto();
  friend void protobuf_ShutdownFile_chirp_5f0000000a_2eproto();

  void InitAsDefaultInstance();
  static ScatterMessage* default_instance_;
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

  // accessors -------------------------------------------------------

  // optional uint64 value = 2;
  void clear_value();
  static const int kValueFieldNumber = 2;
  ::google::protobuf::uint64 value() const;
  void set_value(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:chirp_0000000a_ns.GatherMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::uint64 value_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_chirp_5f0000000a_2eproto();
  friend void protobuf_AssignDesc_chirp_5f0000000a_2eproto();
  friend void protobuf_ShutdownFile_chirp_5f0000000a_2eproto();

  void InitAsDefaultInstance();
  static GatherMessage* default_instance_;
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

  // accessors -------------------------------------------------------

  // optional uint64 value = 2;
  void clear_value();
  static const int kValueFieldNumber = 2;
  ::google::protobuf::uint64 value() const;
  void set_value(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:chirp_0000000a_ns.PublishMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::uint64 value_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_chirp_5f0000000a_2eproto();
  friend void protobuf_AssignDesc_chirp_5f0000000a_2eproto();
  friend void protobuf_ShutdownFile_chirp_5f0000000a_2eproto();

  void InitAsDefaultInstance();
  static PublishMessage* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// ScatterMessage

// -------------------------------------------------------------------

// GatherMessage

// optional uint64 value = 2;
inline void GatherMessage::clear_value() {
  value_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 GatherMessage::value() const {
  // @@protoc_insertion_point(field_get:chirp_0000000a_ns.GatherMessage.value)
  return value_;
}
inline void GatherMessage::set_value(::google::protobuf::uint64 value) {
  
  value_ = value;
  // @@protoc_insertion_point(field_set:chirp_0000000a_ns.GatherMessage.value)
}

// -------------------------------------------------------------------

// PublishMessage

// optional uint64 value = 2;
inline void PublishMessage::clear_value() {
  value_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 PublishMessage::value() const {
  // @@protoc_insertion_point(field_get:chirp_0000000a_ns.PublishMessage.value)
  return value_;
}
inline void PublishMessage::set_value(::google::protobuf::uint64 value) {
  
  value_ = value;
  // @@protoc_insertion_point(field_set:chirp_0000000a_ns.PublishMessage.value)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace chirp_0000000a_ns

struct chirp_0000000a
{
    typedef chirp_0000000a_ns::PublishMessage PublishMessage;
    typedef chirp_0000000a_ns::ScatterMessage ScatterMessage;
    typedef chirp_0000000a_ns::GatherMessage  GatherMessage;

    enum {
        SIGNATURE = 0x0000000a
    };
};

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_chirp_5f0000000a_2eproto__INCLUDED
