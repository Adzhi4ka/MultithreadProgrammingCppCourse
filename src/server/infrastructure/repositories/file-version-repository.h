#pragma once

#include "infrastructure/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/file-version.h"

#include <cstdint>

namespace infrastructure::repositories {

    class FileVersionRepository {

            using WriteUnitOfWork = infrastructure::db::sqlite::WriteUnitOfWork;
            using ReadUnitOfWork = infrastructure::db::sqlite::ReadUnitOfWork;
            using UnitOfWork = infrastructure::db::sqlite::UnitOfWork;
            using FileVersion = domain::models::FileVersion;

        public:

            PersistenceResult<void> addVersion(WriteUnitOfWork& wuov, FileVersion version);
            PersistenceResult<void> updateName(WriteUnitOfWork& wuov, int64_t versionId, const std::string& name);
            PersistenceResult<void> removeOldVersions(WriteUnitOfWork& wuov, int64_t fileId, int32_t keepLastN);

            PersistenceResult<std::vector<FileVersion>> getVersions(UnitOfWork& uow, int64_t fileId);
            PersistenceResult<FileVersion> getVersion(UnitOfWork& uow, int64_t versionId);
            PersistenceResult<int64_t> getVersionId(UnitOfWork& uow, int64_t fileId, int32_t version);
    };

}