#pragma once

#include "infrastructure/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/file-acl.h"

#include <cstdint>

namespace infrastructure::repositories {

    class FileAclRepository {

            using WriteUnitOfWork = infrastructure::db::sqlite::WriteUnitOfWork;
            using ReadUnitOfWork = infrastructure::db::sqlite::ReadUnitOfWork;
            using UnitOfWork = infrastructure::db::sqlite::UnitOfWork;
            using FileAcl = domain::models::FileAcl;
            using AclLevel = domain::models::AclLevel;

        public:

            PersistenceResult<void> grant(WriteUnitOfWork& wuov, FileAcl fileAcl);
            PersistenceResult<void> revoke(WriteUnitOfWork& wuov, int64_t fileId, int64_t groupId);

            PersistenceResult<AclLevel> getFileAcl(UnitOfWork& uow, int64_t fileId, int64_t groupId);
            PersistenceResult<std::vector<FileAcl>> getFileAclsToFileId(UnitOfWork& uow, int64_t fileId);
            PersistenceResult<std::vector<FileAcl>> getGroupFileAcls(UnitOfWork& uow, int64_t groupId);

    };

}