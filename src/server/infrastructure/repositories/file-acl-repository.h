#pragma once

#include <cstdint>

#include "domain/models/file-acl.h"
#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/persistence-result.h"

namespace infrastructure::repositories {

class FileAclRepository {

        using WriteUnitOfWork = infrastructure::db::sqlite::WriteUnitOfWork;
        using ReadUnitOfWork = infrastructure::db::sqlite::ReadUnitOfWork;
        using UnitOfWork = infrastructure::db::sqlite::UnitOfWork;
        using FileAcl = domain::models::FileAcl;
        using AclLevel = domain::models::AclLevel;

    public:

        PersistenceResult<void> grant(WriteUnitOfWork& wuow, FileAcl fileAcl);
        PersistenceResult<void> revoke(WriteUnitOfWork& wuow, int64_t fileId, int64_t groupId);

        PersistenceResult<AclLevel> getFileAcl(UnitOfWork& uow, int64_t fileId, int64_t groupId);
        PersistenceResult<std::vector<FileAcl>> getFileAclsToFileId(UnitOfWork& uow, int64_t fileId);
        PersistenceResult<std::vector<FileAcl>> getGroupFileAcls(UnitOfWork& uow, int64_t groupId);
};

}  // namespace infrastructure::repositories