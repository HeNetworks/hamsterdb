/*
 * Copyright (C) 2005-2014 Christoph Rupp (chris@crupp.de).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

/*
 * Abstraction layer for the remote protocol
 */

#ifndef HAM_PROTOCOL_H__
#define HAM_PROTOCOL_H__

#include <ham/hamsterdb.h>
#include "../mem.h"
#include "../error.h"
#include "../util.h"
#include "../endianswap.h"
#include "messages.pb.h"

using namespace hamsterdb;

/** a magic and version indicator for the remote protocol */
#define HAM_TRANSFER_MAGIC_V1   (('h'<<24)|('a'<<16)|('m'<<8)|'1')

/**
 * the Protocol class maps a single message that is exchanged between
 * client and server
 */
class Protocol : public hamsterdb::ProtoWrapper
{
  public:
    Protocol() { }

    /** constructor - assigns a type */
    Protocol(hamsterdb::ProtoWrapper_Type type) {
      set_type(type);
    }

    /** helper function which copies a ham_key_t into a ProtoBuf key */
    static void assign_key(hamsterdb::Key *protokey, ham_key_t *hamkey,
            bool deep_copy = true) {
      if (deep_copy)
        protokey->set_data(hamkey->data, hamkey->size);
      protokey->set_flags(hamkey->flags);
      protokey->set_intflags(hamkey->_flags);
    }

    /** helper function which copies a ham_record_t into a ProtoBuf record */
    static void assign_record(hamsterdb::Record *protorec,
            ham_record_t *hamrec, bool deep_copy = true) {
      if (deep_copy)
        protorec->set_data(hamrec->data, hamrec->size);
      protorec->set_flags(hamrec->flags);
      protorec->set_partial_offset(hamrec->partial_offset);
      protorec->set_partial_size(hamrec->partial_size);
    }

    /**
     * Factory function; creates a new Protocol structure from a serialized
     * buffer
     */
    static Protocol *unpack(const ham_u8_t *buf, ham_u32_t size) {
      if (*(ham_u32_t *)&buf[0] != ham_db2h32(HAM_TRANSFER_MAGIC_V1)) {
        ham_trace(("invalid protocol version"));
        return (0);
      }

      Protocol *p = new Protocol;
      if (!p->ParseFromArray(buf + 8, size - 8)) {
        delete p;
        return (0);
      }
      return (p);
    }

    /*
     * Packs the Protocol structure into a memory buffer and returns
     * a pointer to the buffer and the buffer size
     */
    bool pack(ham_u8_t **data, ham_u32_t *size) {
      ham_u32_t packed_size = ByteSize();
      /* we need 8 more bytes for magic and size */
      ham_u8_t *p = Memory::allocate<ham_u8_t>(packed_size + 8);
      if (!p)
        return (false);

      /* write the magic and the payload size of the packed structure */
      *(ham_u32_t *)&p[0] = ham_h2db32(HAM_TRANSFER_MAGIC_V1);
      *(ham_u32_t *)&p[4] = ham_h2db32(packed_size);

      /* now write the packed structure */
      if (!SerializeToArray(&p[8], packed_size)) {
        Memory::release(p);
        return (false);
      }

      *data = p;
      *size = packed_size + 8;
      return (true);
    }

    /*
     * Packs the Protocol structure into a ByteArray
     */
    bool pack(ByteArray *barray) {
      ham_u32_t packed_size = ByteSize();
      /* we need 8 more bytes for magic and size */
      ham_u8_t *p = (ham_u8_t *)barray->resize(packed_size + 8);
      if (!p)
        return (false);

      /* write the magic and the payload size of the packed structure */
      *(ham_u32_t *)&p[0] = ham_h2db32(HAM_TRANSFER_MAGIC_V1);
      *(ham_u32_t *)&p[4] = ham_h2db32(packed_size);

      /* now write the packed structure */
      return (SerializeToArray(&p[8], packed_size));
    }

    /**
     * shutdown/free globally allocated memory
     */
    static void shutdown() {
      google::protobuf::ShutdownProtobufLibrary();
    }
};

#endif /* HAM_PROTOCOL_H__ */
