#include "service-test-integration.h"

#include <filesystem>

#include "infrastructure/security/password-hasher.h"

namespace tests {

void ServiceIntegrationTest::SetUp() {

    std::cout << "=====HELLO FROM YAROSLAV=====" << std::endl;

    std::filesystem::remove(m_dbPath);

    infrastructure::security::initialize();

    m_factory = std::make_unique<DatabaseFactory>(m_dbPath, 2);
    m_database = std::make_unique<SqliteDatabase>(*m_factory);

    m_userRepo = std::make_unique<UserRepository>();
    m_groupRepo = std::make_unique<GroupRepository>();
    m_userGroupRepo = std::make_unique<UserGroupRepository>();
    m_fileRepo = std::make_unique<FileRepository>();
    m_fileVersionRepo = std::make_unique<FileVersionRepository>();
    m_fileAclRepo = std::make_unique<FileAclRepository>();
    m_fileLockRepo = std::make_unique<FileLockRepository>();

    m_userService = std::make_unique<UserService>(*m_database, *m_userRepo, *m_groupRepo, *m_userGroupRepo);

    m_groupService = std::make_unique<GroupService>(*m_database, *m_groupRepo, *m_userGroupRepo);

    m_fileAclService = std::make_unique<FileAclService>(*m_database, *m_fileAclRepo, *m_userGroupRepo);

    m_fileService = std::make_unique<FileService>(*m_database, *m_fileRepo, *m_fileVersionRepo, *m_userRepo,
                                                  *m_groupRepo, *m_fileAclRepo);

    m_fileVersionService = std::make_unique<FileVersionService>(*m_database, *m_fileRepo, *m_fileVersionRepo);

    m_fileLockService = std::make_unique<FileLockService>(*m_database, *m_fileLockRepo);

    m_fileContentService = std::make_unique<FileContentService>();
}

void ServiceIntegrationTest::TearDown() {
    m_fileContentService.reset();

    m_fileLockService.reset();
    m_fileVersionService.reset();
    m_fileService.reset();
    m_fileAclService.reset();
    m_groupService.reset();
    m_userService.reset();

    m_fileLockRepo.reset();
    m_fileAclRepo.reset();
    m_fileVersionRepo.reset();
    m_fileRepo.reset();
    m_userGroupRepo.reset();
    m_groupRepo.reset();
    m_userRepo.reset();

    m_database.reset();
    m_factory.reset();

    std::filesystem::remove(m_dbPath);
}

}  // namespace tests