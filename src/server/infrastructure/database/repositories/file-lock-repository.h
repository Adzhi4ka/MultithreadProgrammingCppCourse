#pragma once

#include "infrastructure/database/repositories/repository-return-types.h"
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

            RepositoryOpResult<void> lock(WriteUnitOfWork& wuov, FileLock fileLock);
            RepositoryOpResult<void> unlock(WriteUnitOfWork& wuov, int64_t fileId);
            RepositoryOpResult<void> updateLease(WriteUnitOfWork& wuov, int64_t fileId, int64_t leaseUntil);

            RepositoryOpResult<FileLock> getLock(UnitOfWork& wuov, int64_t fileId);

    };

}