/*
 * Copyright (C) 2005-2014 Christoph Rupp (chris@crupp.de).
 * All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains the property
 * of Christoph Rupp and his suppliers, if any. The intellectual and
 * technical concepts contained herein are proprietary to Christoph Rupp
 * and his suppliers and may be covered by Patents, patents in process,
 * and are protected by trade secret or copyright law. Dissemination of
 * this information or reproduction of this material is strictly forbidden
 * unless prior written permission is obtained from Christoph Rupp.
 */

#ifndef HAM_TXN_REMOTE_H__
#define HAM_TXN_REMOTE_H__

#ifdef HAM_ENABLE_REMOTE

#include "txn.h"

namespace hamsterdb {

//
// A remote Transaction
//
class RemoteTransaction : public Transaction
{
  public:
    // Constructor; "begins" the Transaction
    // supported flags: HAM_TXN_READ_ONLY, HAM_TXN_TEMPORARY
    RemoteTransaction(Environment *env, const char *name, ham_u32_t flags);

    // Commits the Transaction
    virtual void commit(ham_u32_t flags = 0);

    // Aborts the Transaction
    virtual void abort(ham_u32_t flags = 0);

    // Returns the remote Transaction handle
    ham_u64_t get_remote_handle() const {
      return (m_remote_handle);
    }

    // Sets the remote Transaction handle
    void set_remote_handle(ham_u64_t handle) {
      m_remote_handle = handle;
    }

  private:
    // The remote Transaction handle
    ham_u64_t m_remote_handle;
};


//
// A TransactionManager for remote Transactions
//
class RemoteTransactionManager : public TransactionManager
{
  public:
    // Constructor
    RemoteTransactionManager(Environment *env)
      : TransactionManager(env) {
    }

    // Begins a new Transaction
    virtual Transaction *begin(const char *name, ham_u32_t flags);

    // Commits a Transaction; the derived subclass has to take care of
    // flushing and/or releasing memory
    virtual void commit(Transaction *txn, ham_u32_t flags = 0);

    // Aborts a Transaction; the derived subclass has to take care of
    // flushing and/or releasing memory
    virtual void abort(Transaction *txn, ham_u32_t flags = 0);

    // Flushes committed (queued) transactions
    virtual void flush_committed_txns();
};


} // namespace hamsterdb

#endif // HAM_ENABLE_REMOTE

#endif /* HAM_TXN_REMOTE_H__ */
