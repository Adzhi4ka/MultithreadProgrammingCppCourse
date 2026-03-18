#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"

#include "infrastructure/database/models/file-acl.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class FileAclRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileAclRepository(SqliteDatabase& db) : m_db(db) {}

            std::future<void> grant(int64_t fileId, int64_t groupId, int aclLevel);

            std::future<void> revoke(int64_t fileId, int64_t groupId);

            std::future<std::vector<FileAcl>> getFileAcl(int64_t fileId);

            std::future<std::vector<FileAcl>> getGroupAcl(int64_t groupId);

    };

}