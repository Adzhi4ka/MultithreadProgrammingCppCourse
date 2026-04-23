#pragma once

#include "domain/models/file-acl.h"
#include "domain/services/service-result.h"

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/file-acl-repository.h"
#include "infrastructure/repositories/user-group-repository.h"

#include <cstdint>
#include <vector>

namespace domain::services {

    class FileAclService {

            using SqliteDatabase = infrastructure::db::sqlite::SqliteDatabase;
            using FileAclRepository = infrastructure::repositories::FileAclRepository;
            using UserGroupRepository = infrastructure::repositories::UserGroupRepository;
            using AclLevel = domain::models::AclLevel;
            using FileAcl = domain::models::FileAcl;

            SqliteDatabase& m_database;
            FileAclRepository& m_fileAclRepo;
            UserGroupRepository& m_userGroupRepo;

        public:

            FileAclService(SqliteDatabase& database,
                           FileAclRepository& fileAclRepo,
                           UserGroupRepository& userGroupRepo) noexcept;

            ServiceResult<AclLevel> getGroupAclLevel(int64_t groupId, int64_t fileId);

            ServiceResult<AclLevel> getUserAclLevel(int64_t userId, int64_t fileId);

            ServiceResult<void> setGroupAclLevel(int64_t fileId, int64_t groupId, AclLevel aclLevel);

            ServiceResult<void> removeGroupAclLevel(int64_t fileId, int64_t groupId);

            ServiceResult<std::vector<FileAcl>> getFileAcls(int64_t fileId);

            ServiceResult<std::vector<FileAcl>> getGroupAcls(int64_t groupId);

    };

}