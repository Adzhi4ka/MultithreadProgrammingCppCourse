#pragma once

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

            void grant(WriteUnitOfWork& wuov, FileAcl fileAcl);
            void revoke(WriteUnitOfWork& wuov, int64_t fileId, int64_t groupId);

            std::vector<FileAcl> getFileAcl(UnitOfWork& readUnitOfWork, int64_t fileId);
            std::vector<FileAcl> getGroupAcl(UnitOfWork& readUnitOfWork, int64_t groupId);

    };

}