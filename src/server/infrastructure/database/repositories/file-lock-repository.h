#pragma once

#include "infrastructure/database/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/file-lock.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class FileLockRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileLockRepository(SqliteDatabase& db)
                : m_db(db) {}

            PersistenceResult<void> lock(WriteUnitOfWork& wuov, FileLock fileLock);
            PersistenceResult<void> unlock(WriteUnitOfWork& wuov, int64_t fileId);
            PersistenceResult<void> updateLease(WriteUnitOfWork& wuov, int64_t fileId, int64_t leaseUntil);

            PersistenceResult<FileLock> getLock(UnitOfWork& wuov, int64_t fileId);

    };

}