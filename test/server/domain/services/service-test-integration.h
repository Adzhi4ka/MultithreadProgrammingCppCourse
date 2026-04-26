#pragma once

#include <gtest/gtest.h>

#include "infrastructure/database/sqlite/database-factory.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "infrastructure/repositories/user-repository.h"
#include "infrastructure/repositories/group-repository.h"
#include "infrastructure/repositories/user-group-repository.h"
#include "infrastructure/repositories/file-repository.h"
#include "infrastructure/repositories/file-version-repository.h"
#include "infrastructure/repositories/file-acl-repository.h"
#include "infrastructure/repositories/file-lock-repository.h"

#include "domain/services/user-service.h"
#include "domain/services/group-service.h"
#include "domain/services/file-acl-service.h"
#include "domain/services/file-service.h"
#include "domain/services/file-version-service.h"
#include "domain/services/file-lock-service.h"
#include "domain/services/file-content-service.h"

#include <memory>
#include <string>

namespace tests {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::repositories;
    using namespace domain::services;

    class ServiceIntegrationTest : public ::testing::Test {

        protected:

            std::string m_dbPath {"service_integration_test.db"};

            std::unique_ptr<DatabaseFactory> m_factory;
            std::unique_ptr<SqliteDatabase> m_database;

            std::unique_ptr<UserRepository> m_userRepo;
            std::unique_ptr<GroupRepository> m_groupRepo;
            std::unique_ptr<UserGroupRepository> m_userGroupRepo;
            std::unique_ptr<FileRepository> m_fileRepo;
            std::unique_ptr<FileVersionRepository> m_fileVersionRepo;
            std::unique_ptr<FileAclRepository> m_fileAclRepo;
            std::unique_ptr<FileLockRepository> m_fileLockRepo;

            std::unique_ptr<UserService> m_userService;
            std::unique_ptr<GroupService> m_groupService;
            std::unique_ptr<FileAclService> m_fileAclService;
            std::unique_ptr<FileService> m_fileService;
            std::unique_ptr<FileVersionService> m_fileVersionService;
            std::unique_ptr<FileLockService> m_fileLockService;
            std::unique_ptr<FileContentService> m_fileContentService;

        protected:

            void SetUp() override;
            void TearDown() override;

    };

} // namespace tests