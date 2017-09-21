// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: yogi_0000000d.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "yogi_0000000d.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
// @@protoc_insertion_point(includes)

namespace yogi_0000000d_ns {

void protobuf_ShutdownFile_yogi_5f0000000d_2eproto() {
  delete ScatterMessage::default_instance_;
  delete GatherMessage::default_instance_;
  delete PublishMessage::default_instance_;
  delete MasterMessage::default_instance_;
  delete SlaveMessage::default_instance_;
}

#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
void protobuf_AddDesc_yogi_5f0000000d_2eproto_impl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#else
void protobuf_AddDesc_yogi_5f0000000d_2eproto() GOOGLE_ATTRIBUTE_COLD;
void protobuf_AddDesc_yogi_5f0000000d_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#endif
  ScatterMessage::default_instance_ = new ScatterMessage();
  GatherMessage::default_instance_ = new GatherMessage();
  PublishMessage::default_instance_ = new PublishMessage();
  MasterMessage::default_instance_ = new MasterMessage();
  SlaveMessage::default_instance_ = new SlaveMessage();
  ScatterMessage::default_instance_->InitAsDefaultInstance();
  GatherMessage::default_instance_->InitAsDefaultInstance();
  PublishMessage::default_instance_->InitAsDefaultInstance();
  MasterMessage::default_instance_->InitAsDefaultInstance();
  SlaveMessage::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_yogi_5f0000000d_2eproto);
}

#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AddDesc_yogi_5f0000000d_2eproto_once_);
void protobuf_AddDesc_yogi_5f0000000d_2eproto() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AddDesc_yogi_5f0000000d_2eproto_once_,
                 &protobuf_AddDesc_yogi_5f0000000d_2eproto_impl);
}
#else
// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_yogi_5f0000000d_2eproto {
  StaticDescriptorInitializer_yogi_5f0000000d_2eproto() {
    protobuf_AddDesc_yogi_5f0000000d_2eproto();
  }
} static_descriptor_initializer_yogi_5f0000000d_2eproto_;
#endif

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

ScatterMessage::ScatterMessage()
  : ::google::protobuf::MessageLite(), _arena_ptr_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:yogi_0000000d_ns.ScatterMessage)
}

void ScatterMessage::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

ScatterMessage::ScatterMessage(const ScatterMessage& from)
  : ::google::protobuf::MessageLite(),
    _arena_ptr_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:yogi_0000000d_ns.ScatterMessage)
}

void ScatterMessage::SharedCtor() {
    _is_default_instance_ = false;
  _cached_size_ = 0;
}

ScatterMessage::~ScatterMessage() {
  // @@protoc_insertion_point(destructor:yogi_0000000d_ns.ScatterMessage)
  SharedDtor();
}

void ScatterMessage::SharedDtor() {
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void ScatterMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ScatterMessage& ScatterMessage::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_yogi_5f0000000d_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_yogi_5f0000000d_2eproto();
#endif
  return *default_instance_;
}

ScatterMessage* ScatterMessage::default_instance_ = NULL;

ScatterMessage* ScatterMessage::New(::google::protobuf::Arena* arena) const {
  ScatterMessage* n = new ScatterMessage;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void ScatterMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:yogi_0000000d_ns.ScatterMessage)
}

bool ScatterMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:yogi_0000000d_ns.ScatterMessage)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
  handle_unusual:
    if (tag == 0 ||
        ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
        ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
      goto success;
    }
    DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
  }
success:
  // @@protoc_insertion_point(parse_success:yogi_0000000d_ns.ScatterMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:yogi_0000000d_ns.ScatterMessage)
  return false;
#undef DO_
}

void ScatterMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:yogi_0000000d_ns.ScatterMessage)
  // @@protoc_insertion_point(serialize_end:yogi_0000000d_ns.ScatterMessage)
}

int ScatterMessage::ByteSize() const {
// @@protoc_insertion_point(message_byte_size_start:yogi_0000000d_ns.ScatterMessage)
  int total_size = 0;

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void ScatterMessage::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const ScatterMessage*>(&from));
}

void ScatterMessage::MergeFrom(const ScatterMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:yogi_0000000d_ns.ScatterMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) {
    ::google::protobuf::internal::MergeFromFail(__FILE__, __LINE__);
  }
}

void ScatterMessage::CopyFrom(const ScatterMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:yogi_0000000d_ns.ScatterMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ScatterMessage::IsInitialized() const {

  return true;
}

void ScatterMessage::Swap(ScatterMessage* other) {
  if (other == this) return;
  InternalSwap(other);
}
void ScatterMessage::InternalSwap(ScatterMessage* other) {
  _unknown_fields_.Swap(&other->_unknown_fields_);
  std::swap(_cached_size_, other->_cached_size_);
}

::std::string ScatterMessage::GetTypeName() const {
  return "yogi_0000000d.ScatterMessage";
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// ScatterMessage

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int GatherMessage::kValueFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

GatherMessage::GatherMessage()
  : ::google::protobuf::MessageLite(), _arena_ptr_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:yogi_0000000d_ns.GatherMessage)
}

void GatherMessage::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

GatherMessage::GatherMessage(const GatherMessage& from)
  : ::google::protobuf::MessageLite(),
    _arena_ptr_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:yogi_0000000d_ns.GatherMessage)
}

void GatherMessage::SharedCtor() {
    _is_default_instance_ = false;
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  value_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

GatherMessage::~GatherMessage() {
  // @@protoc_insertion_point(destructor:yogi_0000000d_ns.GatherMessage)
  SharedDtor();
}

void GatherMessage::SharedDtor() {
  value_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void GatherMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const GatherMessage& GatherMessage::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_yogi_5f0000000d_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_yogi_5f0000000d_2eproto();
#endif
  return *default_instance_;
}

GatherMessage* GatherMessage::default_instance_ = NULL;

GatherMessage* GatherMessage::New(::google::protobuf::Arena* arena) const {
  GatherMessage* n = new GatherMessage;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void GatherMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:yogi_0000000d_ns.GatherMessage)
  value_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

bool GatherMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:yogi_0000000d_ns.GatherMessage)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional string value = 2;
      case 2: {
        if (tag == 18) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_value()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->value().data(), this->value().length(),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "yogi_0000000d.GatherMessage.value"));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:yogi_0000000d_ns.GatherMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:yogi_0000000d_ns.GatherMessage)
  return false;
#undef DO_
}

void GatherMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:yogi_0000000d_ns.GatherMessage)
  // optional string value = 2;
  if (this->value().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->value().data(), this->value().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "yogi_0000000d.GatherMessage.value");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->value(), output);
  }

  // @@protoc_insertion_point(serialize_end:yogi_0000000d_ns.GatherMessage)
}

int GatherMessage::ByteSize() const {
// @@protoc_insertion_point(message_byte_size_start:yogi_0000000d_ns.GatherMessage)
  int total_size = 0;

  // optional string value = 2;
  if (this->value().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->value());
  }

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void GatherMessage::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const GatherMessage*>(&from));
}

void GatherMessage::MergeFrom(const GatherMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:yogi_0000000d_ns.GatherMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) {
    ::google::protobuf::internal::MergeFromFail(__FILE__, __LINE__);
  }
  if (from.value().size() > 0) {

    value_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.value_);
  }
}

void GatherMessage::CopyFrom(const GatherMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:yogi_0000000d_ns.GatherMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool GatherMessage::IsInitialized() const {

  return true;
}

void GatherMessage::Swap(GatherMessage* other) {
  if (other == this) return;
  InternalSwap(other);
}
void GatherMessage::InternalSwap(GatherMessage* other) {
  value_.Swap(&other->value_);
  _unknown_fields_.Swap(&other->_unknown_fields_);
  std::swap(_cached_size_, other->_cached_size_);
}

::std::string GatherMessage::GetTypeName() const {
  return "yogi_0000000d.GatherMessage";
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// GatherMessage

// optional string value = 2;
void GatherMessage::clear_value() {
  value_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 const ::std::string& GatherMessage::value() const {
  // @@protoc_insertion_point(field_get:yogi_0000000d_ns.GatherMessage.value)
  return value_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void GatherMessage::set_value(const ::std::string& value) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:yogi_0000000d_ns.GatherMessage.value)
}
 void GatherMessage::set_value(const char* value) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:yogi_0000000d_ns.GatherMessage.value)
}
 void GatherMessage::set_value(const char* value, size_t size) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:yogi_0000000d_ns.GatherMessage.value)
}
 ::std::string* GatherMessage::mutable_value() {
  
  // @@protoc_insertion_point(field_mutable:yogi_0000000d_ns.GatherMessage.value)
  return value_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 ::std::string* GatherMessage::release_value() {
  // @@protoc_insertion_point(field_release:yogi_0000000d_ns.GatherMessage.value)
  
  return value_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void GatherMessage::set_allocated_value(::std::string* value) {
  if (value != NULL) {
    
  } else {
    
  }
  value_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set_allocated:yogi_0000000d_ns.GatherMessage.value)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int PublishMessage::kValueFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

PublishMessage::PublishMessage()
  : ::google::protobuf::MessageLite(), _arena_ptr_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:yogi_0000000d_ns.PublishMessage)
}

void PublishMessage::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

PublishMessage::PublishMessage(const PublishMessage& from)
  : ::google::protobuf::MessageLite(),
    _arena_ptr_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:yogi_0000000d_ns.PublishMessage)
}

void PublishMessage::SharedCtor() {
    _is_default_instance_ = false;
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  value_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

PublishMessage::~PublishMessage() {
  // @@protoc_insertion_point(destructor:yogi_0000000d_ns.PublishMessage)
  SharedDtor();
}

void PublishMessage::SharedDtor() {
  value_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void PublishMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const PublishMessage& PublishMessage::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_yogi_5f0000000d_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_yogi_5f0000000d_2eproto();
#endif
  return *default_instance_;
}

PublishMessage* PublishMessage::default_instance_ = NULL;

PublishMessage* PublishMessage::New(::google::protobuf::Arena* arena) const {
  PublishMessage* n = new PublishMessage;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void PublishMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:yogi_0000000d_ns.PublishMessage)
  value_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

bool PublishMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:yogi_0000000d_ns.PublishMessage)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional string value = 2;
      case 2: {
        if (tag == 18) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_value()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->value().data(), this->value().length(),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "yogi_0000000d.PublishMessage.value"));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:yogi_0000000d_ns.PublishMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:yogi_0000000d_ns.PublishMessage)
  return false;
#undef DO_
}

void PublishMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:yogi_0000000d_ns.PublishMessage)
  // optional string value = 2;
  if (this->value().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->value().data(), this->value().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "yogi_0000000d.PublishMessage.value");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->value(), output);
  }

  // @@protoc_insertion_point(serialize_end:yogi_0000000d_ns.PublishMessage)
}

int PublishMessage::ByteSize() const {
// @@protoc_insertion_point(message_byte_size_start:yogi_0000000d_ns.PublishMessage)
  int total_size = 0;

  // optional string value = 2;
  if (this->value().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->value());
  }

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void PublishMessage::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const PublishMessage*>(&from));
}

void PublishMessage::MergeFrom(const PublishMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:yogi_0000000d_ns.PublishMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) {
    ::google::protobuf::internal::MergeFromFail(__FILE__, __LINE__);
  }
  if (from.value().size() > 0) {

    value_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.value_);
  }
}

void PublishMessage::CopyFrom(const PublishMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:yogi_0000000d_ns.PublishMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool PublishMessage::IsInitialized() const {

  return true;
}

void PublishMessage::Swap(PublishMessage* other) {
  if (other == this) return;
  InternalSwap(other);
}
void PublishMessage::InternalSwap(PublishMessage* other) {
  value_.Swap(&other->value_);
  _unknown_fields_.Swap(&other->_unknown_fields_);
  std::swap(_cached_size_, other->_cached_size_);
}

::std::string PublishMessage::GetTypeName() const {
  return "yogi_0000000d.PublishMessage";
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// PublishMessage

// optional string value = 2;
void PublishMessage::clear_value() {
  value_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 const ::std::string& PublishMessage::value() const {
  // @@protoc_insertion_point(field_get:yogi_0000000d_ns.PublishMessage.value)
  return value_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void PublishMessage::set_value(const ::std::string& value) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:yogi_0000000d_ns.PublishMessage.value)
}
 void PublishMessage::set_value(const char* value) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:yogi_0000000d_ns.PublishMessage.value)
}
 void PublishMessage::set_value(const char* value, size_t size) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:yogi_0000000d_ns.PublishMessage.value)
}
 ::std::string* PublishMessage::mutable_value() {
  
  // @@protoc_insertion_point(field_mutable:yogi_0000000d_ns.PublishMessage.value)
  return value_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 ::std::string* PublishMessage::release_value() {
  // @@protoc_insertion_point(field_release:yogi_0000000d_ns.PublishMessage.value)
  
  return value_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void PublishMessage::set_allocated_value(::std::string* value) {
  if (value != NULL) {
    
  } else {
    
  }
  value_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set_allocated:yogi_0000000d_ns.PublishMessage.value)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

MasterMessage::MasterMessage()
  : ::google::protobuf::MessageLite(), _arena_ptr_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:yogi_0000000d_ns.MasterMessage)
}

void MasterMessage::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

MasterMessage::MasterMessage(const MasterMessage& from)
  : ::google::protobuf::MessageLite(),
    _arena_ptr_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:yogi_0000000d_ns.MasterMessage)
}

void MasterMessage::SharedCtor() {
    _is_default_instance_ = false;
  _cached_size_ = 0;
}

MasterMessage::~MasterMessage() {
  // @@protoc_insertion_point(destructor:yogi_0000000d_ns.MasterMessage)
  SharedDtor();
}

void MasterMessage::SharedDtor() {
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void MasterMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const MasterMessage& MasterMessage::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_yogi_5f0000000d_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_yogi_5f0000000d_2eproto();
#endif
  return *default_instance_;
}

MasterMessage* MasterMessage::default_instance_ = NULL;

MasterMessage* MasterMessage::New(::google::protobuf::Arena* arena) const {
  MasterMessage* n = new MasterMessage;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void MasterMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:yogi_0000000d_ns.MasterMessage)
}

bool MasterMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:yogi_0000000d_ns.MasterMessage)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
  handle_unusual:
    if (tag == 0 ||
        ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
        ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
      goto success;
    }
    DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
  }
success:
  // @@protoc_insertion_point(parse_success:yogi_0000000d_ns.MasterMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:yogi_0000000d_ns.MasterMessage)
  return false;
#undef DO_
}

void MasterMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:yogi_0000000d_ns.MasterMessage)
  // @@protoc_insertion_point(serialize_end:yogi_0000000d_ns.MasterMessage)
}

int MasterMessage::ByteSize() const {
// @@protoc_insertion_point(message_byte_size_start:yogi_0000000d_ns.MasterMessage)
  int total_size = 0;

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void MasterMessage::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const MasterMessage*>(&from));
}

void MasterMessage::MergeFrom(const MasterMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:yogi_0000000d_ns.MasterMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) {
    ::google::protobuf::internal::MergeFromFail(__FILE__, __LINE__);
  }
}

void MasterMessage::CopyFrom(const MasterMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:yogi_0000000d_ns.MasterMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool MasterMessage::IsInitialized() const {

  return true;
}

void MasterMessage::Swap(MasterMessage* other) {
  if (other == this) return;
  InternalSwap(other);
}
void MasterMessage::InternalSwap(MasterMessage* other) {
  _unknown_fields_.Swap(&other->_unknown_fields_);
  std::swap(_cached_size_, other->_cached_size_);
}

::std::string MasterMessage::GetTypeName() const {
  return "yogi_0000000d.MasterMessage";
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// MasterMessage

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int SlaveMessage::kValueFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

SlaveMessage::SlaveMessage()
  : ::google::protobuf::MessageLite(), _arena_ptr_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:yogi_0000000d_ns.SlaveMessage)
}

void SlaveMessage::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

SlaveMessage::SlaveMessage(const SlaveMessage& from)
  : ::google::protobuf::MessageLite(),
    _arena_ptr_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:yogi_0000000d_ns.SlaveMessage)
}

void SlaveMessage::SharedCtor() {
    _is_default_instance_ = false;
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  value_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

SlaveMessage::~SlaveMessage() {
  // @@protoc_insertion_point(destructor:yogi_0000000d_ns.SlaveMessage)
  SharedDtor();
}

void SlaveMessage::SharedDtor() {
  value_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void SlaveMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const SlaveMessage& SlaveMessage::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_yogi_5f0000000d_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_yogi_5f0000000d_2eproto();
#endif
  return *default_instance_;
}

SlaveMessage* SlaveMessage::default_instance_ = NULL;

SlaveMessage* SlaveMessage::New(::google::protobuf::Arena* arena) const {
  SlaveMessage* n = new SlaveMessage;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void SlaveMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:yogi_0000000d_ns.SlaveMessage)
  value_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

bool SlaveMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:yogi_0000000d_ns.SlaveMessage)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional string value = 2;
      case 2: {
        if (tag == 18) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_value()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->value().data(), this->value().length(),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "yogi_0000000d.SlaveMessage.value"));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:yogi_0000000d_ns.SlaveMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:yogi_0000000d_ns.SlaveMessage)
  return false;
#undef DO_
}

void SlaveMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:yogi_0000000d_ns.SlaveMessage)
  // optional string value = 2;
  if (this->value().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->value().data(), this->value().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "yogi_0000000d.SlaveMessage.value");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->value(), output);
  }

  // @@protoc_insertion_point(serialize_end:yogi_0000000d_ns.SlaveMessage)
}

int SlaveMessage::ByteSize() const {
// @@protoc_insertion_point(message_byte_size_start:yogi_0000000d_ns.SlaveMessage)
  int total_size = 0;

  // optional string value = 2;
  if (this->value().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->value());
  }

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void SlaveMessage::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const SlaveMessage*>(&from));
}

void SlaveMessage::MergeFrom(const SlaveMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:yogi_0000000d_ns.SlaveMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) {
    ::google::protobuf::internal::MergeFromFail(__FILE__, __LINE__);
  }
  if (from.value().size() > 0) {

    value_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.value_);
  }
}

void SlaveMessage::CopyFrom(const SlaveMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:yogi_0000000d_ns.SlaveMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool SlaveMessage::IsInitialized() const {

  return true;
}

void SlaveMessage::Swap(SlaveMessage* other) {
  if (other == this) return;
  InternalSwap(other);
}
void SlaveMessage::InternalSwap(SlaveMessage* other) {
  value_.Swap(&other->value_);
  _unknown_fields_.Swap(&other->_unknown_fields_);
  std::swap(_cached_size_, other->_cached_size_);
}

::std::string SlaveMessage::GetTypeName() const {
  return "yogi_0000000d.SlaveMessage";
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// SlaveMessage

// optional string value = 2;
void SlaveMessage::clear_value() {
  value_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 const ::std::string& SlaveMessage::value() const {
  // @@protoc_insertion_point(field_get:yogi_0000000d_ns.SlaveMessage.value)
  return value_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void SlaveMessage::set_value(const ::std::string& value) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:yogi_0000000d_ns.SlaveMessage.value)
}
 void SlaveMessage::set_value(const char* value) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:yogi_0000000d_ns.SlaveMessage.value)
}
 void SlaveMessage::set_value(const char* value, size_t size) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:yogi_0000000d_ns.SlaveMessage.value)
}
 ::std::string* SlaveMessage::mutable_value() {
  
  // @@protoc_insertion_point(field_mutable:yogi_0000000d_ns.SlaveMessage.value)
  return value_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 ::std::string* SlaveMessage::release_value() {
  // @@protoc_insertion_point(field_release:yogi_0000000d_ns.SlaveMessage.value)
  
  return value_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void SlaveMessage::set_allocated_value(::std::string* value) {
  if (value != NULL) {
    
  } else {
    
  }
  value_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set_allocated:yogi_0000000d_ns.SlaveMessage.value)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace yogi_0000000d_ns

// @@protoc_insertion_point(global_scope)
