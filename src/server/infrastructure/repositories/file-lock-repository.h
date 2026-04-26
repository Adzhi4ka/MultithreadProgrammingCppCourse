#pragma once

#include "infrastructure/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/file-lock.h"

#include <cstdint>

namespace infrastructure::repositories {

    class FileLockRepository {

            using WriteUnitOfWork = infrastructure::db::sqlite::WriteUnitOfWork;
            using ReadUnitOfWork = infrastructure::db::sqlite::ReadUnitOfWork;
            using UnitOfWork = infrastructure::db::sqlite::UnitOfWork;
            using FileLock = domain::models::FileLock;

        public:

            PersistenceResult<void> lock(WriteUnitOfWork& wuow, FileLock file);
            PersistenceResult<void> unlock(WriteUnitOfWork& wuow, int64_t fileId, int64_t lockToken);
            PersistenceResult<void> updateLease(WriteUnitOfWork& wuow, int64_t fileId, int64_t lockToken, int64_t leaseUntil);

            PersistenceResult<FileLock> getLock(UnitOfWork& uow, int64_t fileId);
    };

}