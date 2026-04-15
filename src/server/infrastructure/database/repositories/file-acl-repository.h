#pragma once

#include "infrastructure/database/repositories/repository-return-types.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/file-acl.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class FileAclRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileAclRepository(SqliteDatabase& db) : m_db(db) {}

            RepositoryOpResult<void> grant(WriteUnitOfWork& wuov, FileAcl fileAcl);
            RepositoryOpResult<void> revoke(WriteUnitOfWork& wuov, int64_t fileId, int64_t groupId);

            RepositoryOpResult<AclLevel> getFileAcl(UnitOfWork& uow, int64_t fileId, int64_t groupId);
            RepositoryOpResult<std::vector<FileAcl>> getFileAclsToFileId(UnitOfWork& uow, int64_t fileId);
            RepositoryOpResult<std::vector<FileAcl>> getGroupFileAcls(UnitOfWork& uow, int64_t groupId);

    };

}