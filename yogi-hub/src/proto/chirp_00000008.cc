// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: chirp_00000008.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "chirp_00000008.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace chirp_00000008_ns {

namespace {

const ::google::protobuf::Descriptor* ScatterMessage_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  ScatterMessage_reflection_ = NULL;
const ::google::protobuf::Descriptor* GatherMessage_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  GatherMessage_reflection_ = NULL;
const ::google::protobuf::Descriptor* PublishMessage_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  PublishMessage_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_chirp_5f00000008_2eproto() {
  protobuf_AddDesc_chirp_5f00000008_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "chirp_00000008.proto");
  GOOGLE_CHECK(file != NULL);
  ScatterMessage_descriptor_ = file->message_type(0);
  static const int ScatterMessage_offsets_[1] = {
  };
  ScatterMessage_reflection_ =
    ::google::protobuf::internal::GeneratedMessageReflection::NewGeneratedMessageReflection(
      ScatterMessage_descriptor_,
      ScatterMessage::default_instance_,
      ScatterMessage_offsets_,
      -1,
      -1,
      -1,
      sizeof(ScatterMessage),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ScatterMessage, _internal_metadata_),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ScatterMessage, _is_default_instance_));
  GatherMessage_descriptor_ = file->message_type(1);
  static const int GatherMessage_offsets_[1] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(GatherMessage, value_),
  };
  GatherMessage_reflection_ =
    ::google::protobuf::internal::GeneratedMessageReflection::NewGeneratedMessageReflection(
      GatherMessage_descriptor_,
      GatherMessage::default_instance_,
      GatherMessage_offsets_,
      -1,
      -1,
      -1,
      sizeof(GatherMessage),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(GatherMessage, _internal_metadata_),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(GatherMessage, _is_default_instance_));
  PublishMessage_descriptor_ = file->message_type(2);
  static const int PublishMessage_offsets_[1] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PublishMessage, value_),
  };
  PublishMessage_reflection_ =
    ::google::protobuf::internal::GeneratedMessageReflection::NewGeneratedMessageReflection(
      PublishMessage_descriptor_,
      PublishMessage::default_instance_,
      PublishMessage_offsets_,
      -1,
      -1,
      -1,
      sizeof(PublishMessage),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PublishMessage, _internal_metadata_),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PublishMessage, _is_default_instance_));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_chirp_5f00000008_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
      ScatterMessage_descriptor_, &ScatterMessage::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
      GatherMessage_descriptor_, &GatherMessage::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
      PublishMessage_descriptor_, &PublishMessage::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_chirp_5f00000008_2eproto() {
  delete ScatterMessage::default_instance_;
  delete ScatterMessage_reflection_;
  delete GatherMessage::default_instance_;
  delete GatherMessage_reflection_;
  delete PublishMessage::default_instance_;
  delete PublishMessage_reflection_;
}

void protobuf_AddDesc_chirp_5f00000008_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\024chirp_00000008.proto\022\016chirp_00000008\"\020"
    "\n\016ScatterMessage\"\036\n\rGatherMessage\022\r\n\005val"
    "ue\030\002 \001(\r\"\037\n\016PublishMessage\022\r\n\005value\030\002 \001("
    "\rb\006proto3", 129);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "chirp_00000008.proto", &protobuf_RegisterTypes);
  ScatterMessage::default_instance_ = new ScatterMessage();
  GatherMessage::default_instance_ = new GatherMessage();
  PublishMessage::default_instance_ = new PublishMessage();
  ScatterMessage::default_instance_->InitAsDefaultInstance();
  GatherMessage::default_instance_->InitAsDefaultInstance();
  PublishMessage::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_chirp_5f00000008_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_chirp_5f00000008_2eproto {
  StaticDescriptorInitializer_chirp_5f00000008_2eproto() {
    protobuf_AddDesc_chirp_5f00000008_2eproto();
  }
} static_descriptor_initializer_chirp_5f00000008_2eproto_;

namespace {

static void MergeFromFail(int line) GOOGLE_ATTRIBUTE_COLD;
static void MergeFromFail(int line) {
  GOOGLE_CHECK(false) << __FILE__ << ":" << line;
}

}  // namespace


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

ScatterMessage::ScatterMessage()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:chirp_00000008_ns.ScatterMessage)
}

void ScatterMessage::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

ScatterMessage::ScatterMessage(const ScatterMessage& from)
  : ::google::protobuf::Message(),
    _internal_metadata_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:chirp_00000008_ns.ScatterMessage)
}

void ScatterMessage::SharedCtor() {
    _is_default_instance_ = false;
  _cached_size_ = 0;
}

ScatterMessage::~ScatterMessage() {
  // @@protoc_insertion_point(destructor:chirp_00000008_ns.ScatterMessage)
  SharedDtor();
}

void ScatterMessage::SharedDtor() {
  if (this != default_instance_) {
  }
}

void ScatterMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* ScatterMessage::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return ScatterMessage_descriptor_;
}

const ScatterMessage& ScatterMessage::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_chirp_5f00000008_2eproto();
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
// @@protoc_insertion_point(message_clear_start:chirp_00000008_ns.ScatterMessage)
}

bool ScatterMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:chirp_00000008_ns.ScatterMessage)
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
  // @@protoc_insertion_point(parse_success:chirp_00000008_ns.ScatterMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:chirp_00000008_ns.ScatterMessage)
  return false;
#undef DO_
}

void ScatterMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:chirp_00000008_ns.ScatterMessage)
  // @@protoc_insertion_point(serialize_end:chirp_00000008_ns.ScatterMessage)
}

::google::protobuf::uint8* ScatterMessage::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:chirp_00000008_ns.ScatterMessage)
  // @@protoc_insertion_point(serialize_to_array_end:chirp_00000008_ns.ScatterMessage)
  return target;
}

int ScatterMessage::ByteSize() const {
// @@protoc_insertion_point(message_byte_size_start:chirp_00000008_ns.ScatterMessage)
  int total_size = 0;

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void ScatterMessage::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:chirp_00000008_ns.ScatterMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  const ScatterMessage* source = 
      ::google::protobuf::internal::DynamicCastToGenerated<const ScatterMessage>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:chirp_00000008_ns.ScatterMessage)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:chirp_00000008_ns.ScatterMessage)
    MergeFrom(*source);
  }
}

void ScatterMessage::MergeFrom(const ScatterMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:chirp_00000008_ns.ScatterMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
}

void ScatterMessage::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:chirp_00000008_ns.ScatterMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void ScatterMessage::CopyFrom(const ScatterMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:chirp_00000008_ns.ScatterMessage)
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
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata ScatterMessage::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = ScatterMessage_descriptor_;
  metadata.reflection = ScatterMessage_reflection_;
  return metadata;
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// ScatterMessage

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int GatherMessage::kValueFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

GatherMessage::GatherMessage()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:chirp_00000008_ns.GatherMessage)
}

void GatherMessage::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

GatherMessage::GatherMessage(const GatherMessage& from)
  : ::google::protobuf::Message(),
    _internal_metadata_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:chirp_00000008_ns.GatherMessage)
}

void GatherMessage::SharedCtor() {
    _is_default_instance_ = false;
  _cached_size_ = 0;
  value_ = 0u;
}

GatherMessage::~GatherMessage() {
  // @@protoc_insertion_point(destructor:chirp_00000008_ns.GatherMessage)
  SharedDtor();
}

void GatherMessage::SharedDtor() {
  if (this != default_instance_) {
  }
}

void GatherMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* GatherMessage::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return GatherMessage_descriptor_;
}

const GatherMessage& GatherMessage::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_chirp_5f00000008_2eproto();
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
// @@protoc_insertion_point(message_clear_start:chirp_00000008_ns.GatherMessage)
  value_ = 0u;
}

bool GatherMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:chirp_00000008_ns.GatherMessage)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional uint32 value = 2;
      case 2: {
        if (tag == 16) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &value_)));

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
  // @@protoc_insertion_point(parse_success:chirp_00000008_ns.GatherMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:chirp_00000008_ns.GatherMessage)
  return false;
#undef DO_
}

void GatherMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:chirp_00000008_ns.GatherMessage)
  // optional uint32 value = 2;
  if (this->value() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->value(), output);
  }

  // @@protoc_insertion_point(serialize_end:chirp_00000008_ns.GatherMessage)
}

::google::protobuf::uint8* GatherMessage::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:chirp_00000008_ns.GatherMessage)
  // optional uint32 value = 2;
  if (this->value() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(2, this->value(), target);
  }

  // @@protoc_insertion_point(serialize_to_array_end:chirp_00000008_ns.GatherMessage)
  return target;
}

int GatherMessage::ByteSize() const {
// @@protoc_insertion_point(message_byte_size_start:chirp_00000008_ns.GatherMessage)
  int total_size = 0;

  // optional uint32 value = 2;
  if (this->value() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->value());
  }

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void GatherMessage::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:chirp_00000008_ns.GatherMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  const GatherMessage* source = 
      ::google::protobuf::internal::DynamicCastToGenerated<const GatherMessage>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:chirp_00000008_ns.GatherMessage)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:chirp_00000008_ns.GatherMessage)
    MergeFrom(*source);
  }
}

void GatherMessage::MergeFrom(const GatherMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:chirp_00000008_ns.GatherMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  if (from.value() != 0) {
    set_value(from.value());
  }
}

void GatherMessage::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:chirp_00000008_ns.GatherMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void GatherMessage::CopyFrom(const GatherMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:chirp_00000008_ns.GatherMessage)
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
  std::swap(value_, other->value_);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata GatherMessage::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = GatherMessage_descriptor_;
  metadata.reflection = GatherMessage_reflection_;
  return metadata;
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// GatherMessage

// optional uint32 value = 2;
void GatherMessage::clear_value() {
  value_ = 0u;
}
 ::google::protobuf::uint32 GatherMessage::value() const {
  // @@protoc_insertion_point(field_get:chirp_00000008_ns.GatherMessage.value)
  return value_;
}
 void GatherMessage::set_value(::google::protobuf::uint32 value) {
  
  value_ = value;
  // @@protoc_insertion_point(field_set:chirp_00000008_ns.GatherMessage.value)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int PublishMessage::kValueFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

PublishMessage::PublishMessage()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:chirp_00000008_ns.PublishMessage)
}

void PublishMessage::InitAsDefaultInstance() {
  _is_default_instance_ = true;
}

PublishMessage::PublishMessage(const PublishMessage& from)
  : ::google::protobuf::Message(),
    _internal_metadata_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:chirp_00000008_ns.PublishMessage)
}

void PublishMessage::SharedCtor() {
    _is_default_instance_ = false;
  _cached_size_ = 0;
  value_ = 0u;
}

PublishMessage::~PublishMessage() {
  // @@protoc_insertion_point(destructor:chirp_00000008_ns.PublishMessage)
  SharedDtor();
}

void PublishMessage::SharedDtor() {
  if (this != default_instance_) {
  }
}

void PublishMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* PublishMessage::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return PublishMessage_descriptor_;
}

const PublishMessage& PublishMessage::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_chirp_5f00000008_2eproto();
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
// @@protoc_insertion_point(message_clear_start:chirp_00000008_ns.PublishMessage)
  value_ = 0u;
}

bool PublishMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:chirp_00000008_ns.PublishMessage)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional uint32 value = 2;
      case 2: {
        if (tag == 16) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &value_)));

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
  // @@protoc_insertion_point(parse_success:chirp_00000008_ns.PublishMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:chirp_00000008_ns.PublishMessage)
  return false;
#undef DO_
}

void PublishMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:chirp_00000008_ns.PublishMessage)
  // optional uint32 value = 2;
  if (this->value() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->value(), output);
  }

  // @@protoc_insertion_point(serialize_end:chirp_00000008_ns.PublishMessage)
}

::google::protobuf::uint8* PublishMessage::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:chirp_00000008_ns.PublishMessage)
  // optional uint32 value = 2;
  if (this->value() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(2, this->value(), target);
  }

  // @@protoc_insertion_point(serialize_to_array_end:chirp_00000008_ns.PublishMessage)
  return target;
}

int PublishMessage::ByteSize() const {
// @@protoc_insertion_point(message_byte_size_start:chirp_00000008_ns.PublishMessage)
  int total_size = 0;

  // optional uint32 value = 2;
  if (this->value() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->value());
  }

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void PublishMessage::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:chirp_00000008_ns.PublishMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  const PublishMessage* source = 
      ::google::protobuf::internal::DynamicCastToGenerated<const PublishMessage>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:chirp_00000008_ns.PublishMessage)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:chirp_00000008_ns.PublishMessage)
    MergeFrom(*source);
  }
}

void PublishMessage::MergeFrom(const PublishMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:chirp_00000008_ns.PublishMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  if (from.value() != 0) {
    set_value(from.value());
  }
}

void PublishMessage::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:chirp_00000008_ns.PublishMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void PublishMessage::CopyFrom(const PublishMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:chirp_00000008_ns.PublishMessage)
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
  std::swap(value_, other->value_);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata PublishMessage::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = PublishMessage_descriptor_;
  metadata.reflection = PublishMessage_reflection_;
  return metadata;
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// PublishMessage

// optional uint32 value = 2;
void PublishMessage::clear_value() {
  value_ = 0u;
}
 ::google::protobuf::uint32 PublishMessage::value() const {
  // @@protoc_insertion_point(field_get:chirp_00000008_ns.PublishMessage.value)
  return value_;
}
 void PublishMessage::set_value(::google::protobuf::uint32 value) {
  
  value_ = value;
  // @@protoc_insertion_point(field_set:chirp_00000008_ns.PublishMessage.value)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace chirp_00000008_ns

// @@protoc_insertion_point(global_scope)
