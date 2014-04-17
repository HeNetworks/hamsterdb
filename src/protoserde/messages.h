/*
 * Copyright (C) 2005-2014 Christoph Rupp (chris@crupp.de).
 *
 * Licensed under the Apache License
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HAM_MESSAGES_H__
#define HAM_MESSAGES_H__

#include <ham/hamsterdb.h>
#include <assert.h>

/** a magic and version indicator for the remote protocol */
#define HAM_TRANSFER_MAGIC_V2   (('h'<<24)|('a'<<16)|('m'<<8)|'2')

namespace hamsterdb {

enum {
  kConnectRequest,
  kConnectReply,
  kDisconnectRequest,
  kDisconnectReply,
  kEnvRenameRequest,
  kEnvRenameReply,
  kEnvGetParametersRequest,
  kEnvGetParametersReply,
  kEnvGetDatabaseNamesRequest,
  kEnvGetDatabaseNamesReply,
  kEnvFlushRequest,
  kEnvFlushReply,
  kEnvCreateDbRequest,
  kEnvCreateDbReply,
  kEnvOpenDbRequest,
  kEnvOpenDbReply,
  kEnvEraseDbRequest,
  kEnvEraseDbReply,
  kDbCloseRequest,
  kDbCloseReply,
  kDbGetParametersRequest,
  kDbGetParametersReply,
  kTxnBeginRequest,
  kTxnBeginReply,
  kTxnCommitRequest,
  kTxnCommitReply,
  kTxnAbortRequest,
  kTxnAbortReply,
  kDbCheckIntegrityRequest,
  kDbCheckIntegrityReply,
  kDbGetKeyCountRequest,
  kDbGetKeyCountReply,
  kDbInsertRequest,
  kDbInsertReply,
  kDbEraseRequest,
  kDbEraseReply,
  kDbFindRequest,
  kDbFindReply,
  kCursorCreateRequest,
  kCursorCreateReply,
  kCursorCloneRequest,
  kCursorCloneReply,
  kCursorCloseRequest,
  kCursorCloseReply,
  kCursorInsertRequest,
  kCursorInsertReply,
  kCursorEraseRequest,
  kCursorEraseReply,
  kCursorFindRequest,
  kCursorFindReply,
  kCursorGetRecordCountRequest,
  kCursorGetRecordCountReply,
  kCursorOverwriteRequest,
  kCursorOverwriteReply,
  kCursorMoveRequest,
  kCursorMoveReply
};

template<typename Ex, typename In>
struct Serialized_Base {
  Ex value;

  Serialized_Base() {
    clear();
  }

  Serialized_Base(const Ex &t)
    : value((In)t) {
  }

  operator Ex() {
    return (value);
  }

  void clear() {
    value = (Ex)0;
  }

  size_t get_size() const {
    return (sizeof(In));
  }

  void serialize(unsigned char **pptr, int *psize) const {
    *(In *)*pptr = (In)value;
    *pptr += sizeof(In);
    *psize -= sizeof(In);
    assert(*psize >= 0);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    value = (Ex) *(In *)*pptr;
    *pptr += sizeof(In);
    *psize -= sizeof(In);
    assert(*psize >= 0);
  }
};

struct SerializedBytes {
  ham_u8_t *value;
  ham_u32_t size;

  SerializedBytes() {
    clear();
  }

  size_t align(size_t s) const {
    if (s % 4) return (s + 4 - (s % 4));
    return (s);
  }

  void clear() {
    value = 0; size = 0;
  }

  size_t get_size() const {
    return (align(size)); // align to 32bits
  }

  void serialize(unsigned char **pptr, int *psize) const {
    *(ham_u32_t *)*pptr = size;
    *pptr += sizeof(ham_u32_t);
    if (size) {
      memcpy(*pptr, value, size);
      *pptr += align(size); // align to 32bits
      *psize -= align(size);
      assert(*psize >= 0);
    }
  }

  void deserialize(unsigned char **pptr, int *psize) {
    size = *(ham_u32_t *)*pptr;
    *pptr += sizeof(ham_u32_t);
    if (size) {
      value = *pptr;
      *pptr += align(size); // align to 32bits
      *psize -= align(size);
      assert(*psize >= 0);
    }
    else
      value = 0;
  }
};

typedef Serialized_Base<bool, ham_u32_t> SerializedBool;
typedef Serialized_Base<ham_u8_t, ham_u32_t> SerializedUint8;
typedef Serialized_Base<ham_u16_t, ham_u32_t> SerializedUint16;
typedef Serialized_Base<ham_u32_t, ham_u32_t> SerializedUint32;
typedef Serialized_Base<ham_s8_t, ham_s32_t> SerializedSint8;
typedef Serialized_Base<ham_s16_t, ham_s32_t> SerializedSint16;
typedef Serialized_Base<ham_s32_t, ham_s32_t> SerializedSint32;
typedef Serialized_Base<ham_u64_t, ham_u64_t> SerializedUint64;
typedef Serialized_Base<ham_s64_t, ham_s64_t> SerializedSint64;


struct SerializedKey {
  SerializedBool has_data;
  SerializedBytes data;
  SerializedUint32 flags;
  SerializedUint32 intflags;

  SerializedKey() {
    clear();
  }

  size_t get_size() const {
    return (
          has_data.get_size() + 
          (has_data.value ? data.get_size() : 0) + 
          flags.get_size() + 
          intflags.get_size() + 
          0);
  }

  void clear() {
    has_data = false;
    data.clear();
    flags.clear();
    intflags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    has_data.serialize(pptr, psize);
    if (has_data.value) data.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    intflags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    has_data.deserialize(pptr, psize);
    if (has_data.value) data.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    intflags.deserialize(pptr, psize);
  }
};

struct SerializedRecord {
  SerializedBool has_data;
  SerializedBytes data;
  SerializedUint32 flags;
  SerializedUint32 partial_offset;
  SerializedUint32 partial_size;

  SerializedRecord() {
    clear();
  }

  size_t get_size() const {
    return (
          has_data.get_size() + 
          (has_data.value ? data.get_size() : 0) + 
          flags.get_size() + 
          partial_offset.get_size() + 
          partial_size.get_size() + 
          0);
  }

  void clear() {
    has_data = false;
    data.clear();
    flags.clear();
    partial_offset.clear();
    partial_size.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    has_data.serialize(pptr, psize);
    if (has_data.value) data.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    partial_offset.serialize(pptr, psize);
    partial_size.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    has_data.deserialize(pptr, psize);
    if (has_data.value) data.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    partial_offset.deserialize(pptr, psize);
    partial_size.deserialize(pptr, psize);
  }
};

struct SerializedConnectRequest {
  SerializedBytes path;

  SerializedConnectRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          path.get_size() + 
          0);
  }

  void clear() {
    path.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    path.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    path.deserialize(pptr, psize);
  }
};

struct SerializedConnectReply {
  SerializedSint32 status;
  SerializedUint32 env_flags;
  SerializedUint64 env_handle;

  SerializedConnectReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          env_flags.get_size() + 
          env_handle.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    env_flags.clear();
    env_handle.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    env_flags.serialize(pptr, psize);
    env_handle.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    env_flags.deserialize(pptr, psize);
    env_handle.deserialize(pptr, psize);
  }
};

struct SerializedDisconnectRequest {
  SerializedUint64 handle;

  SerializedDisconnectRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          handle.get_size() + 
          0);
  }

  void clear() {
    handle.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    handle.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    handle.deserialize(pptr, psize);
  }
};

struct SerializedDisconnectReply {
  SerializedSint32 status;

  SerializedDisconnectReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedEnvGetParametersRequest {
  SerializedUint64 env_handle;
  SerializedUint32 names;

  SerializedEnvGetParametersRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          env_handle.get_size() + 
          names.get_size() + 
          0);
  }

  void clear() {
    env_handle.clear();
    names.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    env_handle.serialize(pptr, psize);
    names.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    env_handle.deserialize(pptr, psize);
    names.deserialize(pptr, psize);
  }
};

struct SerializedEnvGetParametersReply {
  SerializedSint32 status;
  SerializedUint32 cache_size;
  SerializedUint32 page_size;
  SerializedUint32 max_env_databases;
  SerializedUint32 flags;
  SerializedUint32 filemode;
  SerializedBytes filename;

  SerializedEnvGetParametersReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          cache_size.get_size() + 
          page_size.get_size() + 
          max_env_databases.get_size() + 
          flags.get_size() + 
          filemode.get_size() + 
          filename.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    cache_size.clear();
    page_size.clear();
    max_env_databases.clear();
    flags.clear();
    filemode.clear();
    filename.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    cache_size.serialize(pptr, psize);
    page_size.serialize(pptr, psize);
    max_env_databases.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    filemode.serialize(pptr, psize);
    filename.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    cache_size.deserialize(pptr, psize);
    page_size.deserialize(pptr, psize);
    max_env_databases.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    filemode.deserialize(pptr, psize);
    filename.deserialize(pptr, psize);
  }
};

struct SerializedEnvGetDatabaseNamesRequest {
  SerializedUint64 env_handle;

  SerializedEnvGetDatabaseNamesRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          env_handle.get_size() + 
          0);
  }

  void clear() {
    env_handle.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    env_handle.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    env_handle.deserialize(pptr, psize);
  }
};

struct SerializedEnvGetDatabaseNamesReply {
  SerializedSint32 status;
  SerializedUint32 names;

  SerializedEnvGetDatabaseNamesReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          names.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    names.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    names.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    names.deserialize(pptr, psize);
  }
};

struct SerializedEnvRenameRequest {
  SerializedUint64 env_handle;
  SerializedUint32 oldname;
  SerializedUint32 newname;
  SerializedUint32 flags;

  SerializedEnvRenameRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          env_handle.get_size() + 
          oldname.get_size() + 
          newname.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    env_handle.clear();
    oldname.clear();
    newname.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    env_handle.serialize(pptr, psize);
    oldname.serialize(pptr, psize);
    newname.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    env_handle.deserialize(pptr, psize);
    oldname.deserialize(pptr, psize);
    newname.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedEnvRenameReply {
  SerializedSint32 status;

  SerializedEnvRenameReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedEnvFlushRequest {
  SerializedUint64 env_handle;
  SerializedUint32 flags;

  SerializedEnvFlushRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          env_handle.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    env_handle.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    env_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    env_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedEnvFlushReply {
  SerializedSint32 status;

  SerializedEnvFlushReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedEnvCreateDbRequest {
  SerializedUint64 env_handle;
  SerializedUint32 dbname;
  SerializedUint32 flags;
  SerializedUint32 param_names;
  SerializedUint64 param_values;

  SerializedEnvCreateDbRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          env_handle.get_size() + 
          dbname.get_size() + 
          flags.get_size() + 
          param_names.get_size() + 
          param_values.get_size() + 
          0);
  }

  void clear() {
    env_handle.clear();
    dbname.clear();
    flags.clear();
    param_names.clear();
    param_values.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    env_handle.serialize(pptr, psize);
    dbname.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    param_names.serialize(pptr, psize);
    param_values.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    env_handle.deserialize(pptr, psize);
    dbname.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    param_names.deserialize(pptr, psize);
    param_values.deserialize(pptr, psize);
  }
};

struct SerializedEnvCreateDbReply {
  SerializedSint32 status;
  SerializedUint64 db_handle;
  SerializedUint32 db_flags;

  SerializedEnvCreateDbReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          db_handle.get_size() + 
          db_flags.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    db_handle.clear();
    db_flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    db_handle.serialize(pptr, psize);
    db_flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    db_handle.deserialize(pptr, psize);
    db_flags.deserialize(pptr, psize);
  }
};

struct SerializedEnvOpenDbRequest {
  SerializedUint64 env_handle;
  SerializedUint32 dbname;
  SerializedUint32 flags;
  SerializedUint32 param_names;
  SerializedUint64 param_values;

  SerializedEnvOpenDbRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          env_handle.get_size() + 
          dbname.get_size() + 
          flags.get_size() + 
          param_names.get_size() + 
          param_values.get_size() + 
          0);
  }

  void clear() {
    env_handle.clear();
    dbname.clear();
    flags.clear();
    param_names.clear();
    param_values.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    env_handle.serialize(pptr, psize);
    dbname.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    param_names.serialize(pptr, psize);
    param_values.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    env_handle.deserialize(pptr, psize);
    dbname.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    param_names.deserialize(pptr, psize);
    param_values.deserialize(pptr, psize);
  }
};

struct SerializedEnvOpenDbReply {
  SerializedSint32 status;
  SerializedUint64 db_handle;
  SerializedUint32 db_flags;

  SerializedEnvOpenDbReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          db_handle.get_size() + 
          db_flags.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    db_handle.clear();
    db_flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    db_handle.serialize(pptr, psize);
    db_flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    db_handle.deserialize(pptr, psize);
    db_flags.deserialize(pptr, psize);
  }
};

struct SerializedEnvEraseDbRequest {
  SerializedUint64 env_handle;
  SerializedUint32 name;
  SerializedUint32 flags;

  SerializedEnvEraseDbRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          env_handle.get_size() + 
          name.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    env_handle.clear();
    name.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    env_handle.serialize(pptr, psize);
    name.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    env_handle.deserialize(pptr, psize);
    name.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedEnvEraseDbReply {
  SerializedSint32 status;

  SerializedEnvEraseDbReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedDbCloseRequest {
  SerializedUint64 db_handle;
  SerializedUint32 flags;

  SerializedDbCloseRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          db_handle.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    db_handle.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    db_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    db_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedDbCloseReply {
  SerializedSint32 status;

  SerializedDbCloseReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedDbGetParametersRequest {
  SerializedUint64 db_handle;
  SerializedUint32 names;

  SerializedDbGetParametersRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          db_handle.get_size() + 
          names.get_size() + 
          0);
  }

  void clear() {
    db_handle.clear();
    names.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    db_handle.serialize(pptr, psize);
    names.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    db_handle.deserialize(pptr, psize);
    names.deserialize(pptr, psize);
  }
};

struct SerializedDbGetParametersReply {
  SerializedSint32 status;
  SerializedUint32 max_env_databases;
  SerializedUint32 flags;
  SerializedUint32 key_size;
  SerializedUint32 dbname;
  SerializedUint32 keys_per_page;
  SerializedUint32 key_type;
  SerializedUint32 record_size;

  SerializedDbGetParametersReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          max_env_databases.get_size() + 
          flags.get_size() + 
          key_size.get_size() + 
          dbname.get_size() + 
          keys_per_page.get_size() + 
          key_type.get_size() + 
          record_size.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    max_env_databases.clear();
    flags.clear();
    key_size.clear();
    dbname.clear();
    keys_per_page.clear();
    key_type.clear();
    record_size.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    max_env_databases.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    key_size.serialize(pptr, psize);
    dbname.serialize(pptr, psize);
    keys_per_page.serialize(pptr, psize);
    key_type.serialize(pptr, psize);
    record_size.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    max_env_databases.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    key_size.deserialize(pptr, psize);
    dbname.deserialize(pptr, psize);
    keys_per_page.deserialize(pptr, psize);
    key_type.deserialize(pptr, psize);
    record_size.deserialize(pptr, psize);
  }
};

struct SerializedTxnBeginRequest {
  SerializedUint64 env_handle;
  SerializedUint32 flags;
  SerializedBytes name;

  SerializedTxnBeginRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          env_handle.get_size() + 
          flags.get_size() + 
          name.get_size() + 
          0);
  }

  void clear() {
    env_handle.clear();
    flags.clear();
    name.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    env_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    name.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    env_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    name.deserialize(pptr, psize);
  }
};

struct SerializedTxnBeginReply {
  SerializedSint32 status;
  SerializedUint64 txn_handle;

  SerializedTxnBeginReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          txn_handle.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    txn_handle.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    txn_handle.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    txn_handle.deserialize(pptr, psize);
  }
};

struct SerializedTxnCommitRequest {
  SerializedUint64 txn_handle;
  SerializedUint32 flags;

  SerializedTxnCommitRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          txn_handle.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    txn_handle.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    txn_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    txn_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedTxnCommitReply {
  SerializedSint32 status;

  SerializedTxnCommitReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedTxnAbortRequest {
  SerializedUint64 txn_handle;
  SerializedUint32 flags;

  SerializedTxnAbortRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          txn_handle.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    txn_handle.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    txn_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    txn_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedTxnAbortReply {
  SerializedSint32 status;

  SerializedTxnAbortReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedDbCheckIntegrityRequest {
  SerializedUint64 db_handle;
  SerializedUint32 flags;

  SerializedDbCheckIntegrityRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          db_handle.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    db_handle.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    db_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    db_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedDbCheckIntegrityReply {
  SerializedSint32 status;

  SerializedDbCheckIntegrityReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedDbGetKeyCountRequest {
  SerializedUint64 db_handle;
  SerializedUint64 txn_handle;
  SerializedUint32 flags;

  SerializedDbGetKeyCountRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          db_handle.get_size() + 
          txn_handle.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    db_handle.clear();
    txn_handle.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    db_handle.serialize(pptr, psize);
    txn_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    db_handle.deserialize(pptr, psize);
    txn_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedDbGetKeyCountReply {
  SerializedSint32 status;
  SerializedUint64 keycount;

  SerializedDbGetKeyCountReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          keycount.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    keycount.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    keycount.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    keycount.deserialize(pptr, psize);
  }
};

struct SerializedDbInsertRequest {
  SerializedUint64 db_handle;
  SerializedUint64 txn_handle;
  SerializedUint32 flags;
  SerializedBool has_key;
  SerializedKey key;
  SerializedBool has_record;
  SerializedRecord record;

  SerializedDbInsertRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          db_handle.get_size() + 
          txn_handle.get_size() + 
          flags.get_size() + 
          has_key.get_size() + 
          (has_key.value ? key.get_size() : 0) + 
          has_record.get_size() + 
          (has_record.value ? record.get_size() : 0) + 
          0);
  }

  void clear() {
    db_handle.clear();
    txn_handle.clear();
    flags.clear();
    has_key = false;
    key.clear();
    has_record = false;
    record.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    db_handle.serialize(pptr, psize);
    txn_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    has_key.serialize(pptr, psize);
    if (has_key.value) key.serialize(pptr, psize);
    has_record.serialize(pptr, psize);
    if (has_record.value) record.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    db_handle.deserialize(pptr, psize);
    txn_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    has_key.deserialize(pptr, psize);
    if (has_key.value) key.deserialize(pptr, psize);
    has_record.deserialize(pptr, psize);
    if (has_record.value) record.deserialize(pptr, psize);
  }
};

struct SerializedDbInsertReply {
  SerializedSint32 status;
  SerializedBool has_key;
  SerializedKey key;

  SerializedDbInsertReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          has_key.get_size() + 
          (has_key.value ? key.get_size() : 0) + 
          0);
  }

  void clear() {
    status.clear();
    has_key = false;
    key.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    has_key.serialize(pptr, psize);
    if (has_key.value) key.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    has_key.deserialize(pptr, psize);
    if (has_key.value) key.deserialize(pptr, psize);
  }
};

struct SerializedDbEraseRequest {
  SerializedUint64 db_handle;
  SerializedUint64 txn_handle;
  SerializedKey key;
  SerializedUint32 flags;

  SerializedDbEraseRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          db_handle.get_size() + 
          txn_handle.get_size() + 
          key.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    db_handle.clear();
    txn_handle.clear();
    key.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    db_handle.serialize(pptr, psize);
    txn_handle.serialize(pptr, psize);
    key.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    db_handle.deserialize(pptr, psize);
    txn_handle.deserialize(pptr, psize);
    key.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedDbEraseReply {
  SerializedSint32 status;

  SerializedDbEraseReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedDbFindRequest {
  SerializedUint64 db_handle;
  SerializedUint64 txn_handle;
  SerializedUint32 flags;
  SerializedKey key;
  SerializedBool has_record;
  SerializedRecord record;

  SerializedDbFindRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          db_handle.get_size() + 
          txn_handle.get_size() + 
          flags.get_size() + 
          key.get_size() + 
          has_record.get_size() + 
          (has_record.value ? record.get_size() : 0) + 
          0);
  }

  void clear() {
    db_handle.clear();
    txn_handle.clear();
    flags.clear();
    key.clear();
    has_record = false;
    record.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    db_handle.serialize(pptr, psize);
    txn_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    key.serialize(pptr, psize);
    has_record.serialize(pptr, psize);
    if (has_record.value) record.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    db_handle.deserialize(pptr, psize);
    txn_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    key.deserialize(pptr, psize);
    has_record.deserialize(pptr, psize);
    if (has_record.value) record.deserialize(pptr, psize);
  }
};

struct SerializedDbFindReply {
  SerializedSint32 status;
  SerializedBool has_key;
  SerializedKey key;
  SerializedBool has_record;
  SerializedRecord record;

  SerializedDbFindReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          has_key.get_size() + 
          (has_key.value ? key.get_size() : 0) + 
          has_record.get_size() + 
          (has_record.value ? record.get_size() : 0) + 
          0);
  }

  void clear() {
    status.clear();
    has_key = false;
    key.clear();
    has_record = false;
    record.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    has_key.serialize(pptr, psize);
    if (has_key.value) key.serialize(pptr, psize);
    has_record.serialize(pptr, psize);
    if (has_record.value) record.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    has_key.deserialize(pptr, psize);
    if (has_key.value) key.deserialize(pptr, psize);
    has_record.deserialize(pptr, psize);
    if (has_record.value) record.deserialize(pptr, psize);
  }
};

struct SerializedCursorCreateRequest {
  SerializedUint64 db_handle;
  SerializedUint64 txn_handle;
  SerializedUint32 flags;

  SerializedCursorCreateRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          db_handle.get_size() + 
          txn_handle.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    db_handle.clear();
    txn_handle.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    db_handle.serialize(pptr, psize);
    txn_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    db_handle.deserialize(pptr, psize);
    txn_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedCursorCreateReply {
  SerializedSint32 status;
  SerializedUint64 cursor_handle;

  SerializedCursorCreateReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          cursor_handle.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    cursor_handle.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    cursor_handle.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    cursor_handle.deserialize(pptr, psize);
  }
};

struct SerializedCursorCloneRequest {
  SerializedUint64 cursor_handle;

  SerializedCursorCloneRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          cursor_handle.get_size() + 
          0);
  }

  void clear() {
    cursor_handle.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    cursor_handle.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    cursor_handle.deserialize(pptr, psize);
  }
};

struct SerializedCursorCloneReply {
  SerializedSint32 status;
  SerializedUint64 cursor_handle;

  SerializedCursorCloneReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          cursor_handle.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    cursor_handle.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    cursor_handle.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    cursor_handle.deserialize(pptr, psize);
  }
};

struct SerializedCursorCloseRequest {
  SerializedUint64 cursor_handle;

  SerializedCursorCloseRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          cursor_handle.get_size() + 
          0);
  }

  void clear() {
    cursor_handle.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    cursor_handle.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    cursor_handle.deserialize(pptr, psize);
  }
};

struct SerializedCursorCloseReply {
  SerializedSint32 status;

  SerializedCursorCloseReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedCursorInsertRequest {
  SerializedUint64 cursor_handle;
  SerializedUint32 flags;
  SerializedBool has_key;
  SerializedKey key;
  SerializedBool has_record;
  SerializedRecord record;

  SerializedCursorInsertRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          cursor_handle.get_size() + 
          flags.get_size() + 
          has_key.get_size() + 
          (has_key.value ? key.get_size() : 0) + 
          has_record.get_size() + 
          (has_record.value ? record.get_size() : 0) + 
          0);
  }

  void clear() {
    cursor_handle.clear();
    flags.clear();
    has_key = false;
    key.clear();
    has_record = false;
    record.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    cursor_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    has_key.serialize(pptr, psize);
    if (has_key.value) key.serialize(pptr, psize);
    has_record.serialize(pptr, psize);
    if (has_record.value) record.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    cursor_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    has_key.deserialize(pptr, psize);
    if (has_key.value) key.deserialize(pptr, psize);
    has_record.deserialize(pptr, psize);
    if (has_record.value) record.deserialize(pptr, psize);
  }
};

struct SerializedCursorInsertReply {
  SerializedSint32 status;
  SerializedBool has_key;
  SerializedKey key;

  SerializedCursorInsertReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          has_key.get_size() + 
          (has_key.value ? key.get_size() : 0) + 
          0);
  }

  void clear() {
    status.clear();
    has_key = false;
    key.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    has_key.serialize(pptr, psize);
    if (has_key.value) key.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    has_key.deserialize(pptr, psize);
    if (has_key.value) key.deserialize(pptr, psize);
  }
};

struct SerializedCursorEraseRequest {
  SerializedUint64 cursor_handle;
  SerializedUint32 flags;

  SerializedCursorEraseRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          cursor_handle.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    cursor_handle.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    cursor_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    cursor_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedCursorEraseReply {
  SerializedSint32 status;

  SerializedCursorEraseReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedCursorFindRequest {
  SerializedUint64 cursor_handle;
  SerializedUint32 flags;
  SerializedKey key;
  SerializedBool has_record;
  SerializedRecord record;

  SerializedCursorFindRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          cursor_handle.get_size() + 
          flags.get_size() + 
          key.get_size() + 
          has_record.get_size() + 
          (has_record.value ? record.get_size() : 0) + 
          0);
  }

  void clear() {
    cursor_handle.clear();
    flags.clear();
    key.clear();
    has_record = false;
    record.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    cursor_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    key.serialize(pptr, psize);
    has_record.serialize(pptr, psize);
    if (has_record.value) record.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    cursor_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    key.deserialize(pptr, psize);
    has_record.deserialize(pptr, psize);
    if (has_record.value) record.deserialize(pptr, psize);
  }
};

struct SerializedCursorFindReply {
  SerializedSint32 status;
  SerializedBool has_key;
  SerializedKey key;
  SerializedBool has_record;
  SerializedRecord record;

  SerializedCursorFindReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          has_key.get_size() + 
          (has_key.value ? key.get_size() : 0) + 
          has_record.get_size() + 
          (has_record.value ? record.get_size() : 0) + 
          0);
  }

  void clear() {
    status.clear();
    has_key = false;
    key.clear();
    has_record = false;
    record.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    has_key.serialize(pptr, psize);
    if (has_key.value) key.serialize(pptr, psize);
    has_record.serialize(pptr, psize);
    if (has_record.value) record.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    has_key.deserialize(pptr, psize);
    if (has_key.value) key.deserialize(pptr, psize);
    has_record.deserialize(pptr, psize);
    if (has_record.value) record.deserialize(pptr, psize);
  }
};

struct SerializedCursorGetRecordCountRequest {
  SerializedUint64 cursor_handle;
  SerializedUint32 flags;

  SerializedCursorGetRecordCountRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          cursor_handle.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    cursor_handle.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    cursor_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    cursor_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedCursorGetRecordCountReply {
  SerializedSint32 status;
  SerializedUint32 count;

  SerializedCursorGetRecordCountReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          count.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    count.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    count.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    count.deserialize(pptr, psize);
  }
};

struct SerializedCursorOverwriteRequest {
  SerializedUint64 cursor_handle;
  SerializedRecord record;
  SerializedUint32 flags;

  SerializedCursorOverwriteRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          cursor_handle.get_size() + 
          record.get_size() + 
          flags.get_size() + 
          0);
  }

  void clear() {
    cursor_handle.clear();
    record.clear();
    flags.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    cursor_handle.serialize(pptr, psize);
    record.serialize(pptr, psize);
    flags.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    cursor_handle.deserialize(pptr, psize);
    record.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
  }
};

struct SerializedCursorOverwriteReply {
  SerializedSint32 status;

  SerializedCursorOverwriteReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          0);
  }

  void clear() {
    status.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
  }
};

struct SerializedCursorMoveRequest {
  SerializedUint64 cursor_handle;
  SerializedUint32 flags;
  SerializedBool has_key;
  SerializedKey key;
  SerializedBool has_record;
  SerializedRecord record;

  SerializedCursorMoveRequest() {
    clear();
  }

  size_t get_size() const {
    return (
          cursor_handle.get_size() + 
          flags.get_size() + 
          has_key.get_size() + 
          (has_key.value ? key.get_size() : 0) + 
          has_record.get_size() + 
          (has_record.value ? record.get_size() : 0) + 
          0);
  }

  void clear() {
    cursor_handle.clear();
    flags.clear();
    has_key = false;
    key.clear();
    has_record = false;
    record.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    cursor_handle.serialize(pptr, psize);
    flags.serialize(pptr, psize);
    has_key.serialize(pptr, psize);
    if (has_key.value) key.serialize(pptr, psize);
    has_record.serialize(pptr, psize);
    if (has_record.value) record.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    cursor_handle.deserialize(pptr, psize);
    flags.deserialize(pptr, psize);
    has_key.deserialize(pptr, psize);
    if (has_key.value) key.deserialize(pptr, psize);
    has_record.deserialize(pptr, psize);
    if (has_record.value) record.deserialize(pptr, psize);
  }
};

struct SerializedCursorMoveReply {
  SerializedSint32 status;
  SerializedKey key;
  SerializedRecord record;

  SerializedCursorMoveReply() {
    clear();
  }

  size_t get_size() const {
    return (
          status.get_size() + 
          key.get_size() + 
          record.get_size() + 
          0);
  }

  void clear() {
    status.clear();
    key.clear();
    record.clear();
  }

  void serialize(unsigned char **pptr, int *psize) const {
    status.serialize(pptr, psize);
    key.serialize(pptr, psize);
    record.serialize(pptr, psize);
  }

  void deserialize(unsigned char **pptr, int *psize) {
    status.deserialize(pptr, psize);
    key.deserialize(pptr, psize);
    record.deserialize(pptr, psize);
  }
};

struct SerializedWrapper {
  SerializedUint32 magic;
  SerializedUint32 size;
  SerializedUint32 id;
  SerializedConnectRequest connect_request;
  SerializedConnectReply connect_reply;
  SerializedDisconnectRequest disconnect_request;
  SerializedDisconnectReply disconnect_reply;
  SerializedEnvRenameRequest env_rename_request;
  SerializedEnvRenameReply env_rename_reply;
  SerializedEnvGetParametersRequest env_get_parameters_request;
  SerializedEnvGetParametersReply env_get_parameters_reply;
  SerializedEnvGetDatabaseNamesRequest env_get_database_names_request;
  SerializedEnvGetDatabaseNamesReply env_get_database_names_reply;
  SerializedEnvFlushRequest env_flush_request;
  SerializedEnvFlushReply env_flush_reply;
  SerializedEnvCreateDbRequest env_create_db_request;
  SerializedEnvCreateDbReply env_create_db_reply;
  SerializedEnvOpenDbRequest env_open_db_request;
  SerializedEnvOpenDbReply env_open_db_reply;
  SerializedEnvEraseDbRequest env_erase_db_request;
  SerializedEnvEraseDbReply env_erase_db_reply;
  SerializedDbCloseRequest db_close_request;
  SerializedDbCloseReply db_close_reply;
  SerializedDbGetParametersRequest db_get_parameters_request;
  SerializedDbGetParametersReply db_get_parameters_reply;
  SerializedTxnBeginRequest txn_begin_request;
  SerializedTxnBeginReply txn_begin_reply;
  SerializedTxnCommitRequest txn_commit_request;
  SerializedTxnCommitReply txn_commit_reply;
  SerializedTxnAbortRequest txn_abort_request;
  SerializedTxnAbortReply txn_abort_reply;
  SerializedDbCheckIntegrityRequest db_check_integrity_request;
  SerializedDbCheckIntegrityReply db_check_integrity_reply;
  SerializedDbGetKeyCountRequest db_get_key_count_request;
  SerializedDbGetKeyCountReply db_get_key_count_reply;
  SerializedDbInsertRequest db_insert_request;
  SerializedDbInsertReply db_insert_reply;
  SerializedDbEraseRequest db_erase_request;
  SerializedDbEraseReply db_erase_reply;
  SerializedDbFindRequest db_find_request;
  SerializedDbFindReply db_find_reply;
  SerializedCursorCreateRequest cursor_create_request;
  SerializedCursorCreateReply cursor_create_reply;
  SerializedCursorCloneRequest cursor_clone_request;
  SerializedCursorCloneReply cursor_clone_reply;
  SerializedCursorCloseRequest cursor_close_request;
  SerializedCursorCloseReply cursor_close_reply;
  SerializedCursorInsertRequest cursor_insert_request;
  SerializedCursorInsertReply cursor_insert_reply;
  SerializedCursorEraseRequest cursor_erase_request;
  SerializedCursorEraseReply cursor_erase_reply;
  SerializedCursorFindRequest cursor_find_request;
  SerializedCursorFindReply cursor_find_reply;
  SerializedCursorGetRecordCountRequest cursor_get_record_count_request;
  SerializedCursorGetRecordCountReply cursor_get_record_count_reply;
  SerializedCursorOverwriteRequest cursor_overwrite_request;
  SerializedCursorOverwriteReply cursor_overwrite_reply;
  SerializedCursorMoveRequest cursor_move_request;
  SerializedCursorMoveReply cursor_move_reply;

  SerializedWrapper() {
    clear();
  }

  // the methods in here have a custom implementation, otherwise we would
  // generate many bools for the "optional" fields, and they would
  // unnecessarily increase the structure size
  void clear() {
    magic = 0;
    size = 0;
    id = 0;
  }

  size_t get_size() const {
    size_t s = magic.get_size() + size.get_size() + id.get_size();
    switch (id.value) {
      case kConnectRequest:
        return (s + connect_request.get_size());
      case kConnectReply:
        return (s + connect_reply.get_size());
      case kDisconnectRequest:
        return (s + disconnect_request.get_size());
      case kDisconnectReply:
        return (s + disconnect_reply.get_size());
      case kEnvRenameRequest:
        return (s + env_rename_request.get_size());
      case kEnvRenameReply:
        return (s + env_rename_reply.get_size());
      case kEnvGetParametersRequest:
        return (s + env_get_parameters_request.get_size());
      case kEnvGetParametersReply:
        return (s + env_get_parameters_reply.get_size());
      case kEnvGetDatabaseNamesRequest:
        return (s + env_get_database_names_request.get_size());
      case kEnvGetDatabaseNamesReply:
        return (s + env_get_database_names_reply.get_size());
      case kEnvCreateDbRequest:
        return (s + env_create_db_request.get_size());
      case kEnvCreateDbReply: 
        return (s + env_create_db_reply.get_size());
      case kEnvOpenDbRequest: 
        return (s + env_open_db_request.get_size());
      case kEnvOpenDbReply: 
        return (s + env_open_db_reply.get_size());
      case kEnvEraseDbRequest: 
        return (s + env_erase_db_request.get_size());
      case kEnvEraseDbReply: 
        return (s + env_erase_db_reply.get_size());
      case kDbCloseRequest: 
        return (s + db_close_request.get_size());
      case kDbCloseReply: 
        return (s + db_close_reply.get_size());
      case kDbGetParametersRequest: 
        return (s + db_get_parameters_request.get_size());
      case kDbGetParametersReply: 
        return (s + db_get_parameters_reply.get_size());
      case kTxnBeginRequest: 
        return (s + txn_begin_request.get_size());
      case kTxnBeginReply: 
        return (s + txn_begin_reply.get_size());
      case kTxnCommitRequest: 
        return (s + txn_commit_request.get_size());
      case kTxnCommitReply: 
        return (s + txn_commit_reply.get_size());
      case kTxnAbortRequest: 
        return (s + txn_abort_request.get_size());
      case kTxnAbortReply: 
        return (s + txn_abort_reply.get_size());
      case kDbCheckIntegrityRequest: 
        return (s + db_check_integrity_request.get_size());
      case kDbCheckIntegrityReply: 
        return (s + db_check_integrity_reply.get_size());
      case kDbGetKeyCountRequest: 
        return (s + db_get_key_count_request.get_size());
      case kDbGetKeyCountReply: 
        return (s + db_get_key_count_reply.get_size());
      case kDbInsertRequest: 
        return (s + db_insert_request.get_size());
      case kDbInsertReply: 
        return (s + db_insert_reply.get_size());
      case kDbEraseRequest: 
        return (s + db_erase_request.get_size());
      case kDbEraseReply: 
        return (s + db_erase_reply.get_size());
      case kDbFindRequest: 
        return (s + db_find_request.get_size());
      case kDbFindReply: 
        return (s + db_find_reply.get_size());
      case kCursorCreateRequest: 
        return (s + cursor_create_request.get_size());
      case kCursorCreateReply: 
        return (s + cursor_create_reply.get_size());
      case kCursorCloneRequest: 
        return (s + cursor_clone_request.get_size());
      case kCursorCloneReply: 
        return (s + cursor_clone_reply.get_size());
      case kCursorCloseRequest: 
        return (s + cursor_close_request.get_size());
      case kCursorCloseReply: 
        return (s + cursor_close_reply.get_size());
      case kCursorInsertRequest: 
        return (s + cursor_insert_request.get_size());
      case kCursorInsertReply: 
        return (s + cursor_insert_reply.get_size());
      case kCursorEraseRequest: 
        return (s + cursor_erase_request.get_size());
      case kCursorEraseReply: 
        return (s + cursor_erase_reply.get_size());
      case kCursorFindRequest: 
        return (s + cursor_find_request.get_size());
      case kCursorFindReply: 
        return (s + cursor_find_reply.get_size());
      case kCursorGetRecordCountRequest: 
        return (s + cursor_get_record_count_request.get_size());
      case kCursorGetRecordCountReply: 
        return (s + cursor_get_record_count_reply.get_size());
      case kCursorOverwriteRequest: 
        return (s + cursor_overwrite_request.get_size());
      case kCursorOverwriteReply: 
        return (s + cursor_overwrite_reply.get_size());
      case kCursorMoveRequest: 
        return (s + cursor_move_request.get_size());
      case kCursorMoveReply: 
        return (s + cursor_move_reply.get_size());
      default:
        assert(!"shouldn't be here");
        return (0);
    }
  }

  void serialize(unsigned char **pptr, int *psize) const {
    magic.serialize(pptr, psize);
    size.serialize(pptr, psize);
    id.serialize(pptr, psize);

    switch (id.value) {
      case kConnectRequest:
        connect_request.serialize(pptr, psize);
        break;
      case kConnectReply:
        connect_reply.serialize(pptr, psize);
        break;
      case kDisconnectRequest:
        disconnect_request.serialize(pptr, psize);
        break;
      case kDisconnectReply:
        disconnect_reply.serialize(pptr, psize);
        break;
      case kEnvRenameRequest:
        env_rename_request.serialize(pptr, psize);
        break;
      case kEnvRenameReply:
        env_rename_reply.serialize(pptr, psize);
        break;
      case kEnvGetParametersRequest:
        env_get_parameters_request.serialize(pptr, psize);
        break;
      case kEnvGetParametersReply:
        env_get_parameters_reply.serialize(pptr, psize);
        break;
      case kEnvGetDatabaseNamesRequest:
        env_get_database_names_request.serialize(pptr, psize);
        break;
      case kEnvGetDatabaseNamesReply:
        env_get_database_names_reply.serialize(pptr, psize);
        break;
      case kEnvCreateDbRequest:
        env_create_db_request.serialize(pptr, psize);
        break;
      case kEnvCreateDbReply: 
        env_create_db_reply.serialize(pptr, psize);
        break;
      case kEnvOpenDbRequest: 
        env_open_db_request.serialize(pptr, psize);
        break;
      case kEnvOpenDbReply: 
        env_open_db_reply.serialize(pptr, psize);
        break;
      case kEnvEraseDbRequest: 
        env_erase_db_request.serialize(pptr, psize);
        break;
      case kEnvEraseDbReply: 
        env_erase_db_reply.serialize(pptr, psize);
        break;
      case kDbCloseRequest: 
        db_close_request.serialize(pptr, psize);
        break;
      case kDbCloseReply: 
        db_close_reply.serialize(pptr, psize);
        break;
      case kDbGetParametersRequest: 
        db_get_parameters_request.serialize(pptr, psize);
        break;
      case kDbGetParametersReply: 
        db_get_parameters_reply.serialize(pptr, psize);
        break;
      case kTxnBeginRequest: 
        txn_begin_request.serialize(pptr, psize);
        break;
      case kTxnBeginReply: 
        txn_begin_reply.serialize(pptr, psize);
        break;
      case kTxnCommitRequest: 
        txn_commit_request.serialize(pptr, psize);
        break;
      case kTxnCommitReply: 
        txn_commit_reply.serialize(pptr, psize);
        break;
      case kTxnAbortRequest: 
        txn_abort_request.serialize(pptr, psize);
        break;
      case kTxnAbortReply: 
        txn_abort_reply.serialize(pptr, psize);
        break;
      case kDbCheckIntegrityRequest: 
        db_check_integrity_request.serialize(pptr, psize);
        break;
      case kDbCheckIntegrityReply: 
        db_check_integrity_reply.serialize(pptr, psize);
        break;
      case kDbGetKeyCountRequest: 
        db_get_key_count_request.serialize(pptr, psize);
        break;
      case kDbGetKeyCountReply: 
        db_get_key_count_reply.serialize(pptr, psize);
        break;
      case kDbInsertRequest: 
        db_insert_request.serialize(pptr, psize);
        break;
      case kDbInsertReply: 
        db_insert_reply.serialize(pptr, psize);
        break;
      case kDbEraseRequest: 
        db_erase_request.serialize(pptr, psize);
        break;
      case kDbEraseReply: 
        db_erase_reply.serialize(pptr, psize);
        break;
      case kDbFindRequest: 
        db_find_request.serialize(pptr, psize);
        break;
      case kDbFindReply: 
        db_find_reply.serialize(pptr, psize);
        break;
      case kCursorCreateRequest: 
        cursor_create_request.serialize(pptr, psize);
        break;
      case kCursorCreateReply: 
        cursor_create_reply.serialize(pptr, psize);
        break;
      case kCursorCloneRequest: 
        cursor_clone_request.serialize(pptr, psize);
        break;
      case kCursorCloneReply: 
        cursor_clone_reply.serialize(pptr, psize);
        break;
      case kCursorCloseRequest: 
        cursor_close_request.serialize(pptr, psize);
        break;
      case kCursorCloseReply: 
        cursor_close_reply.serialize(pptr, psize);
        break;
      case kCursorInsertRequest: 
        cursor_insert_request.serialize(pptr, psize);
        break;
      case kCursorInsertReply: 
        cursor_insert_reply.serialize(pptr, psize);
        break;
      case kCursorEraseRequest: 
        cursor_erase_request.serialize(pptr, psize);
        break;
      case kCursorEraseReply: 
        cursor_erase_reply.serialize(pptr, psize);
        break;
      case kCursorFindRequest: 
        cursor_find_request.serialize(pptr, psize);
        break;
      case kCursorFindReply: 
        cursor_find_reply.serialize(pptr, psize);
        break;
      case kCursorGetRecordCountRequest: 
        cursor_get_record_count_request.serialize(pptr, psize);
        break;
      case kCursorGetRecordCountReply: 
        cursor_get_record_count_reply.serialize(pptr, psize);
        break;
      case kCursorOverwriteRequest: 
        cursor_overwrite_request.serialize(pptr, psize);
        break;
      case kCursorOverwriteReply: 
        cursor_overwrite_reply.serialize(pptr, psize);
        break;
      case kCursorMoveRequest: 
        cursor_move_request.serialize(pptr, psize);
        break;
      case kCursorMoveReply: 
        cursor_move_reply.serialize(pptr, psize);
        break;
      default:
        assert(!"shouldn't be here");
    }
  }

  void deserialize(unsigned char **pptr, int *psize) {
    magic.deserialize(pptr, psize);
    size.deserialize(pptr, psize);
    id.deserialize(pptr, psize);

    switch (id.value) {
      case kConnectRequest:
        connect_request.deserialize(pptr, psize);
        break;
      case kConnectReply:
        connect_reply.deserialize(pptr, psize);
        break;
      case kDisconnectRequest:
        disconnect_request.deserialize(pptr, psize);
        break;
      case kDisconnectReply:
        disconnect_reply.deserialize(pptr, psize);
        break;
      case kEnvRenameRequest:
        env_rename_request.deserialize(pptr, psize);
        break;
      case kEnvRenameReply:
        env_rename_reply.deserialize(pptr, psize);
        break;
      case kEnvGetParametersRequest:
        env_get_parameters_request.deserialize(pptr, psize);
        break;
      case kEnvGetParametersReply:
        env_get_parameters_reply.deserialize(pptr, psize);
        break;
      case kEnvGetDatabaseNamesRequest:
        env_get_database_names_request.deserialize(pptr, psize);
        break;
      case kEnvGetDatabaseNamesReply:
        env_get_database_names_reply.deserialize(pptr, psize);
        break;
      case kEnvCreateDbRequest:
        env_create_db_request.deserialize(pptr, psize);
        break;
      case kEnvCreateDbReply: 
        env_create_db_reply.deserialize(pptr, psize);
        break;
      case kEnvOpenDbRequest: 
        env_open_db_request.deserialize(pptr, psize);
        break;
      case kEnvOpenDbReply: 
        env_open_db_reply.deserialize(pptr, psize);
        break;
      case kEnvEraseDbRequest: 
        env_erase_db_request.deserialize(pptr, psize);
        break;
      case kEnvEraseDbReply: 
        env_erase_db_reply.deserialize(pptr, psize);
        break;
      case kDbCloseRequest: 
        db_close_request.deserialize(pptr, psize);
        break;
      case kDbCloseReply: 
        db_close_reply.deserialize(pptr, psize);
        break;
      case kDbGetParametersRequest: 
        db_get_parameters_request.deserialize(pptr, psize);
        break;
      case kDbGetParametersReply: 
        db_get_parameters_reply.deserialize(pptr, psize);
        break;
      case kTxnBeginRequest: 
        txn_begin_request.deserialize(pptr, psize);
        break;
      case kTxnBeginReply: 
        txn_begin_reply.deserialize(pptr, psize);
        break;
      case kTxnCommitRequest: 
        txn_commit_request.deserialize(pptr, psize);
        break;
      case kTxnCommitReply: 
        txn_commit_reply.deserialize(pptr, psize);
        break;
      case kTxnAbortRequest: 
        txn_abort_request.deserialize(pptr, psize);
        break;
      case kTxnAbortReply: 
        txn_abort_reply.deserialize(pptr, psize);
        break;
      case kDbCheckIntegrityRequest: 
        db_check_integrity_request.deserialize(pptr, psize);
        break;
      case kDbCheckIntegrityReply: 
        db_check_integrity_reply.deserialize(pptr, psize);
        break;
      case kDbGetKeyCountRequest: 
        db_get_key_count_request.deserialize(pptr, psize);
        break;
      case kDbGetKeyCountReply: 
        db_get_key_count_reply.deserialize(pptr, psize);
        break;
      case kDbInsertRequest: 
        db_insert_request.deserialize(pptr, psize);
        break;
      case kDbInsertReply: 
        db_insert_reply.deserialize(pptr, psize);
        break;
      case kDbEraseRequest: 
        db_erase_request.deserialize(pptr, psize);
        break;
      case kDbEraseReply: 
        db_erase_reply.deserialize(pptr, psize);
        break;
      case kDbFindRequest: 
        db_find_request.deserialize(pptr, psize);
        break;
      case kDbFindReply: 
        db_find_reply.deserialize(pptr, psize);
        break;
      case kCursorCreateRequest: 
        cursor_create_request.deserialize(pptr, psize);
        break;
      case kCursorCreateReply: 
        cursor_create_reply.deserialize(pptr, psize);
        break;
      case kCursorCloneRequest: 
        cursor_clone_request.deserialize(pptr, psize);
        break;
      case kCursorCloneReply: 
        cursor_clone_reply.deserialize(pptr, psize);
        break;
      case kCursorCloseRequest: 
        cursor_close_request.deserialize(pptr, psize);
        break;
      case kCursorCloseReply: 
        cursor_close_reply.deserialize(pptr, psize);
        break;
      case kCursorInsertRequest: 
        cursor_insert_request.deserialize(pptr, psize);
        break;
      case kCursorInsertReply: 
        cursor_insert_reply.deserialize(pptr, psize);
        break;
      case kCursorEraseRequest: 
        cursor_erase_request.deserialize(pptr, psize);
        break;
      case kCursorEraseReply: 
        cursor_erase_reply.deserialize(pptr, psize);
        break;
      case kCursorFindRequest: 
        cursor_find_request.deserialize(pptr, psize);
        break;
      case kCursorFindReply: 
        cursor_find_reply.deserialize(pptr, psize);
        break;
      case kCursorGetRecordCountRequest: 
        cursor_get_record_count_request.deserialize(pptr, psize);
        break;
      case kCursorGetRecordCountReply: 
        cursor_get_record_count_reply.deserialize(pptr, psize);
        break;
      case kCursorOverwriteRequest: 
        cursor_overwrite_request.deserialize(pptr, psize);
        break;
      case kCursorOverwriteReply: 
        cursor_overwrite_reply.serialize(pptr, psize);
        break;
      case kCursorMoveRequest: 
        cursor_move_request.deserialize(pptr, psize);
        break;
      case kCursorMoveReply: 
        cursor_move_reply.deserialize(pptr, psize);
        break;
      default:
        assert(!"shouldn't be here");
    }
  }
};


} // namespace hamsterdb
#endif // HAM_MESSAGES_H__

