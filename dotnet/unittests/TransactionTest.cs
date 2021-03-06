﻿/*
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

using Hamster;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace Unittests
{
    /// <summary>
    ///This is a test class for TransactionTest and is intended
    ///to contain all TransactionTest Unit Tests
    ///</summary>
    [TestClass()]
    [DeploymentItem("..\\win32\\msvc2008\\out\\dll_debug\\hamsterdb-2.1.7.dll")]
    public class TransactionTest
    {
        private Hamster.Environment env;
        private Database db;

        [TestInitialize()]
        public void MyTestInitialize()
        {
            env = new Hamster.Environment();
            env.Create("test.db", HamConst.HAM_ENABLE_TRANSACTIONS);
            db = env.CreateDatabase(1);
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            db.Close();
            env.Close();
        }

        [DeploymentItem("HamsterDb-dotnet.dll")]

        [TestMethod()]
        public void AbortTest()
        {
            Transaction t = env.Begin();
            t.Abort();
        }

        [TestMethod()]
        public void CommitTest()
        {
            Transaction t = env.Begin();
            t.Commit();
        }

        [TestMethod()]
        public void InsertFindCommitTest() {
            byte[] k = new byte[5];
            byte[] r = new byte[5];
            Transaction t=env.Begin();
            db.Insert(t, k, r);
            db.Find(t, k);
            try {
                db.Find(k);
            }
            catch (DatabaseException e) {
                Assert.AreEqual(HamConst.HAM_TXN_CONFLICT, e.ErrorCode);
            }
            t.Commit();
            db.Find(k);
        }

        [TestMethod()]
        public void InsertFindAbortTest() {
            byte[] k = new byte[5];
            byte[] r = new byte[5];
            Transaction t=env.Begin();
            db.Insert(t, k, r);
            db.Find(t, k);
            t.Abort();
            try {
                db.Find(k);
            }
            catch (DatabaseException e) {
                Assert.AreEqual(HamConst.HAM_KEY_NOT_FOUND, e.ErrorCode);
            }
        }

        [TestMethod()]
        public void EraseFindCommitTest() {
            byte[] k = new byte[5];
            byte[] r = new byte[5];
            Transaction t=env.Begin();
            db.Insert(t, k, r);
            db.Find(t, k);
            try {
                db.Erase(k);
            }
            catch (DatabaseException e) {
                Assert.AreEqual(HamConst.HAM_TXN_CONFLICT, e.ErrorCode);
            }
            t.Commit();
            db.Erase(k);
        }

        [TestMethod()]
        public void CursorTest() {
            Transaction t=env.Begin();
            Cursor c = new Cursor(db, t);
            byte[] k = new byte[5];
            byte[] r = new byte[5];
            c.Insert(k, r);
            db.Find(t, k);
            c.Close();
            t.Commit();
            db.Find(k);
        }

        [TestMethod()]
        public void GetKeyCountTest() {
            Transaction t=env.Begin();

            byte[] k = new byte[5];
            byte[] r = new byte[5];
            Assert.AreEqual(0, db.GetKeyCount());
            db.Insert(t, k, r);
            Assert.AreEqual(1, db.GetKeyCount(t));
            t.Commit();
            Assert.AreEqual(1, db.GetKeyCount());
        }
    }
}
