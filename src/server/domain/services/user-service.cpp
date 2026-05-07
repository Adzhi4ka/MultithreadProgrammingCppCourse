#include "user-service.h"

#include "infrastructure/id-generator/id-generator.h"
#include "infrastructure/security/password-hasher.h"

namespace {
using namespace domain::models;
}

namespace domain::services {

UserService::UserService(SqliteDatabase& database, UserRepository& userRepo, GroupRepository& groupRepo,
                         UserGroupRepository& userGroupRepo) noexcept
    : m_database(database), m_userRepo(userRepo), m_groupRepo(groupRepo), m_userGroupRepo(userGroupRepo) {}

ServiceResult<int64_t> UserService::login(const std::string& login, const std::string& rawPassword) {
    auto ruow = m_database.createReadUnitOfWork();

    auto userResult = m_userRepo.getByLogin(ruow, login);
    if (!userResult) {
        return std::unexpected(mapPersistenceError(userResult.error()));
    }

    ruow.close();

    if (infrastructure::security::verifyPassword(rawPassword, userResult->passwordHash)) {
        return userResult->id;
    }

    return std::unexpected(ServiceError::Forbidden);
}

ServiceResult<int64_t> UserService::addUser(std::string login, std::string rawPassword) {
    auto wuow = m_database.createWriteUnitOfWork();

    const int64_t newUserId = infrastructure::id_generator::generateId();
    const int64_t newGroupId = infrastructure::id_generator::generateId();
    const std::string ownerGroupName = login;

    auto createUserResult =
        m_userRepo.create(wuow, User{.id = newUserId,
                                     .login = std::move(login),
                                     .passwordHash = infrastructure::security::hashPassword(rawPassword)});

    if (!createUserResult) {
        return std::unexpected(mapPersistenceError(createUserResult.error()));
    }

    auto createGroupResult = m_groupRepo.create(wuow, Group{.id = newGroupId, .name = ownerGroupName});

    if (!createGroupResult) {
        return std::unexpected(mapPersistenceError(createGroupResult.error()));
    }

    auto addUserToGroupResult =
        m_userGroupRepo.addUserToGroup(wuow, UserGroup{.userId = newUserId, .groupId = newGroupId});

    if (!addUserToGroupResult) {
        return std::unexpected(mapPersistenceError(addUserToGroupResult.error()));
    }

    wuow.commit();
    return newUserId;
}

ServiceResult<User> UserService::getById(int64_t userId) {
    auto ruow = m_database.createReadUnitOfWork();

    auto userResult = m_userRepo.getById(ruow, userId);
    if (!userResult) {
        return std::unexpected(mapPersistenceError(userResult.error()));
    }

    return *userResult;
}

ServiceResult<User> UserService::getByLogin(const std::string& login) {
    auto ruow = m_database.createReadUnitOfWork();

    auto userResult = m_userRepo.getByLogin(ruow, login);
    if (!userResult) {
        return std::unexpected(mapPersistenceError(userResult.error()));
    }

    return *userResult;
}

}  // namespace domain::services
