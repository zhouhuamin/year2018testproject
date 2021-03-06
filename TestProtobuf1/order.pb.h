// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: order.proto

#ifndef PROTOBUF_order_2eproto__INCLUDED
#define PROTOBUF_order_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3001000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3001000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
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

namespace tutorial {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_order_2eproto();
void protobuf_InitDefaults_order_2eproto();
void protobuf_AssignDesc_order_2eproto();
void protobuf_ShutdownFile_order_2eproto();

class order;

// ===================================================================

class order : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:tutorial.order) */ {
 public:
  order();
  virtual ~order();

  order(const order& from);

  inline order& operator=(const order& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const order& default_instance();

  static const order* internal_default_instance();

  void Swap(order* other);

  // implements Message ----------------------------------------------

  inline order* New() const { return New(NULL); }

  order* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const order& from);
  void MergeFrom(const order& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
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
  void InternalSwap(order* other);
  void UnsafeMergeFrom(const order& from);
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

  // optional int32 action = 1;
  void clear_action();
  static const int kActionFieldNumber = 1;
  ::google::protobuf::int32 action() const;
  void set_action(::google::protobuf::int32 value);

  // optional string serialNo = 2;
  void clear_serialno();
  static const int kSerialNoFieldNumber = 2;
  const ::std::string& serialno() const;
  void set_serialno(const ::std::string& value);
  void set_serialno(const char* value);
  void set_serialno(const char* value, size_t size);
  ::std::string* mutable_serialno();
  ::std::string* release_serialno();
  void set_allocated_serialno(::std::string* serialno);

  // optional string version = 3;
  void clear_version();
  static const int kVersionFieldNumber = 3;
  const ::std::string& version() const;
  void set_version(const ::std::string& value);
  void set_version(const char* value);
  void set_version(const char* value, size_t size);
  ::std::string* mutable_version();
  ::std::string* release_version();
  void set_allocated_version(::std::string* version);

  // optional string operator = 4;
  void clear_operator_();
  static const int kOperatorFieldNumber = 4;
  const ::std::string& operator_() const;
  void set_operator_(const ::std::string& value);
  void set_operator_(const char* value);
  void set_operator_(const char* value, size_t size);
  ::std::string* mutable_operator_();
  ::std::string* release_operator_();
  void set_allocated_operator_(::std::string* operator_);

  // repeated string code = 5;
  int code_size() const;
  void clear_code();
  static const int kCodeFieldNumber = 5;
  const ::std::string& code(int index) const;
  ::std::string* mutable_code(int index);
  void set_code(int index, const ::std::string& value);
  void set_code(int index, const char* value);
  void set_code(int index, const char* value, size_t size);
  ::std::string* add_code();
  void add_code(const ::std::string& value);
  void add_code(const char* value);
  void add_code(const char* value, size_t size);
  const ::google::protobuf::RepeatedPtrField< ::std::string>& code() const;
  ::google::protobuf::RepeatedPtrField< ::std::string>* mutable_code();

  // optional string name = 6;
  void clear_name();
  static const int kNameFieldNumber = 6;
  const ::std::string& name() const;
  void set_name(const ::std::string& value);
  void set_name(const char* value);
  void set_name(const char* value, size_t size);
  ::std::string* mutable_name();
  ::std::string* release_name();
  void set_allocated_name(::std::string* name);

  // optional string price = 7;
  void clear_price();
  static const int kPriceFieldNumber = 7;
  const ::std::string& price() const;
  void set_price(const ::std::string& value);
  void set_price(const char* value);
  void set_price(const char* value, size_t size);
  ::std::string* mutable_price();
  ::std::string* release_price();
  void set_allocated_price(::std::string* price);

  // optional string amount = 8;
  void clear_amount();
  static const int kAmountFieldNumber = 8;
  const ::std::string& amount() const;
  void set_amount(const ::std::string& value);
  void set_amount(const char* value);
  void set_amount(const char* value, size_t size);
  ::std::string* mutable_amount();
  ::std::string* release_amount();
  void set_allocated_amount(::std::string* amount);

  // @@protoc_insertion_point(class_scope:tutorial.order)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedPtrField< ::std::string> code_;
  ::google::protobuf::internal::ArenaStringPtr serialno_;
  ::google::protobuf::internal::ArenaStringPtr version_;
  ::google::protobuf::internal::ArenaStringPtr operator__;
  ::google::protobuf::internal::ArenaStringPtr name_;
  ::google::protobuf::internal::ArenaStringPtr price_;
  ::google::protobuf::internal::ArenaStringPtr amount_;
  ::google::protobuf::int32 action_;
  mutable int _cached_size_;
  friend void  protobuf_InitDefaults_order_2eproto_impl();
  friend void  protobuf_AddDesc_order_2eproto_impl();
  friend void protobuf_AssignDesc_order_2eproto();
  friend void protobuf_ShutdownFile_order_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<order> order_default_instance_;

// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// order

// optional int32 action = 1;
inline void order::clear_action() {
  action_ = 0;
}
inline ::google::protobuf::int32 order::action() const {
  // @@protoc_insertion_point(field_get:tutorial.order.action)
  return action_;
}
inline void order::set_action(::google::protobuf::int32 value) {
  
  action_ = value;
  // @@protoc_insertion_point(field_set:tutorial.order.action)
}

// optional string serialNo = 2;
inline void order::clear_serialno() {
  serialno_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& order::serialno() const {
  // @@protoc_insertion_point(field_get:tutorial.order.serialNo)
  return serialno_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_serialno(const ::std::string& value) {
  
  serialno_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:tutorial.order.serialNo)
}
inline void order::set_serialno(const char* value) {
  
  serialno_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:tutorial.order.serialNo)
}
inline void order::set_serialno(const char* value, size_t size) {
  
  serialno_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:tutorial.order.serialNo)
}
inline ::std::string* order::mutable_serialno() {
  
  // @@protoc_insertion_point(field_mutable:tutorial.order.serialNo)
  return serialno_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* order::release_serialno() {
  // @@protoc_insertion_point(field_release:tutorial.order.serialNo)
  
  return serialno_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_allocated_serialno(::std::string* serialno) {
  if (serialno != NULL) {
    
  } else {
    
  }
  serialno_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), serialno);
  // @@protoc_insertion_point(field_set_allocated:tutorial.order.serialNo)
}

// optional string version = 3;
inline void order::clear_version() {
  version_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& order::version() const {
  // @@protoc_insertion_point(field_get:tutorial.order.version)
  return version_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_version(const ::std::string& value) {
  
  version_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:tutorial.order.version)
}
inline void order::set_version(const char* value) {
  
  version_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:tutorial.order.version)
}
inline void order::set_version(const char* value, size_t size) {
  
  version_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:tutorial.order.version)
}
inline ::std::string* order::mutable_version() {
  
  // @@protoc_insertion_point(field_mutable:tutorial.order.version)
  return version_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* order::release_version() {
  // @@protoc_insertion_point(field_release:tutorial.order.version)
  
  return version_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_allocated_version(::std::string* version) {
  if (version != NULL) {
    
  } else {
    
  }
  version_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), version);
  // @@protoc_insertion_point(field_set_allocated:tutorial.order.version)
}

// optional string operator = 4;
inline void order::clear_operator_() {
  operator__.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& order::operator_() const {
  // @@protoc_insertion_point(field_get:tutorial.order.operator)
  return operator__.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_operator_(const ::std::string& value) {
  
  operator__.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:tutorial.order.operator)
}
inline void order::set_operator_(const char* value) {
  
  operator__.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:tutorial.order.operator)
}
inline void order::set_operator_(const char* value, size_t size) {
  
  operator__.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:tutorial.order.operator)
}
inline ::std::string* order::mutable_operator_() {
  
  // @@protoc_insertion_point(field_mutable:tutorial.order.operator)
  return operator__.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* order::release_operator_() {
  // @@protoc_insertion_point(field_release:tutorial.order.operator)
  
  return operator__.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_allocated_operator_(::std::string* operator_) {
  if (operator_ != NULL) {
    
  } else {
    
  }
  operator__.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), operator_);
  // @@protoc_insertion_point(field_set_allocated:tutorial.order.operator)
}

// repeated string code = 5;
inline int order::code_size() const {
  return code_.size();
}
inline void order::clear_code() {
  code_.Clear();
}
inline const ::std::string& order::code(int index) const {
  // @@protoc_insertion_point(field_get:tutorial.order.code)
  return code_.Get(index);
}
inline ::std::string* order::mutable_code(int index) {
  // @@protoc_insertion_point(field_mutable:tutorial.order.code)
  return code_.Mutable(index);
}
inline void order::set_code(int index, const ::std::string& value) {
  // @@protoc_insertion_point(field_set:tutorial.order.code)
  code_.Mutable(index)->assign(value);
}
inline void order::set_code(int index, const char* value) {
  code_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:tutorial.order.code)
}
inline void order::set_code(int index, const char* value, size_t size) {
  code_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:tutorial.order.code)
}
inline ::std::string* order::add_code() {
  // @@protoc_insertion_point(field_add_mutable:tutorial.order.code)
  return code_.Add();
}
inline void order::add_code(const ::std::string& value) {
  code_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:tutorial.order.code)
}
inline void order::add_code(const char* value) {
  code_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:tutorial.order.code)
}
inline void order::add_code(const char* value, size_t size) {
  code_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:tutorial.order.code)
}
inline const ::google::protobuf::RepeatedPtrField< ::std::string>&
order::code() const {
  // @@protoc_insertion_point(field_list:tutorial.order.code)
  return code_;
}
inline ::google::protobuf::RepeatedPtrField< ::std::string>*
order::mutable_code() {
  // @@protoc_insertion_point(field_mutable_list:tutorial.order.code)
  return &code_;
}

// optional string name = 6;
inline void order::clear_name() {
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& order::name() const {
  // @@protoc_insertion_point(field_get:tutorial.order.name)
  return name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_name(const ::std::string& value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:tutorial.order.name)
}
inline void order::set_name(const char* value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:tutorial.order.name)
}
inline void order::set_name(const char* value, size_t size) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:tutorial.order.name)
}
inline ::std::string* order::mutable_name() {
  
  // @@protoc_insertion_point(field_mutable:tutorial.order.name)
  return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* order::release_name() {
  // @@protoc_insertion_point(field_release:tutorial.order.name)
  
  return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_allocated_name(::std::string* name) {
  if (name != NULL) {
    
  } else {
    
  }
  name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:tutorial.order.name)
}

// optional string price = 7;
inline void order::clear_price() {
  price_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& order::price() const {
  // @@protoc_insertion_point(field_get:tutorial.order.price)
  return price_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_price(const ::std::string& value) {
  
  price_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:tutorial.order.price)
}
inline void order::set_price(const char* value) {
  
  price_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:tutorial.order.price)
}
inline void order::set_price(const char* value, size_t size) {
  
  price_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:tutorial.order.price)
}
inline ::std::string* order::mutable_price() {
  
  // @@protoc_insertion_point(field_mutable:tutorial.order.price)
  return price_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* order::release_price() {
  // @@protoc_insertion_point(field_release:tutorial.order.price)
  
  return price_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_allocated_price(::std::string* price) {
  if (price != NULL) {
    
  } else {
    
  }
  price_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), price);
  // @@protoc_insertion_point(field_set_allocated:tutorial.order.price)
}

// optional string amount = 8;
inline void order::clear_amount() {
  amount_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& order::amount() const {
  // @@protoc_insertion_point(field_get:tutorial.order.amount)
  return amount_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_amount(const ::std::string& value) {
  
  amount_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:tutorial.order.amount)
}
inline void order::set_amount(const char* value) {
  
  amount_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:tutorial.order.amount)
}
inline void order::set_amount(const char* value, size_t size) {
  
  amount_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:tutorial.order.amount)
}
inline ::std::string* order::mutable_amount() {
  
  // @@protoc_insertion_point(field_mutable:tutorial.order.amount)
  return amount_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* order::release_amount() {
  // @@protoc_insertion_point(field_release:tutorial.order.amount)
  
  return amount_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void order::set_allocated_amount(::std::string* amount) {
  if (amount != NULL) {
    
  } else {
    
  }
  amount_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), amount);
  // @@protoc_insertion_point(field_set_allocated:tutorial.order.amount)
}

inline const order* order::internal_default_instance() {
  return &order_default_instance_.get();
}
#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace tutorial

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_order_2eproto__INCLUDED
