#include "group-service.h"

#include "infrastructure/id-generator/id-generator.h"

namespace {
    using namespace domain::models;
}

namespace domain::services {

    GroupService::GroupService(SqliteDatabase& database,
                               GroupRepository& groupRepo,
                               UserGroupRepository& userGroupRepo) noexcept
        : m_database(database),
          m_groupRepo(groupRepo),
          m_userGroupRepo(userGroupRepo) {}

    ServiceResult<int64_t> GroupService::createGroup(const std::string& groupName) {
        auto wuow = m_database.createWriteUnitOfWork();

        int64_t newId = infrastructure::id_generator::generateId();

        auto createResult = m_groupRepo.create(wuow, Group{.id = newId,
                                                           .name = groupName});

        if (!createResult) {
            return std::unexpected(mapPersistenceError(createResult.error()));
        }

        wuow.commit();
        return newId;
    }

    ServiceResult<void> GroupService::deleteGroup(int64_t groupId) {
        auto wuow = m_database.createWriteUnitOfWork();

        auto removeResult = m_groupRepo.remove(wuow, groupId);
        if (!removeResult) {
            return std::unexpected(mapPersistenceError(removeResult.error()));
        }

        wuow.commit();
        return {};
    }

    ServiceResult<void> GroupService::addUserToGroup(int64_t userId, int64_t groupId) {
        auto wuow = m_database.createWriteUnitOfWork();

        auto addResult = m_userGroupRepo.addUserToGroup(wuow, UserGroup{.userId = userId,
                                                                        .groupId = groupId});

        if (!addResult) {
            return std::unexpected(mapPersistenceError(addResult.error()));
        }

        wuow.commit();
        return {};
    }

    ServiceResult<void> GroupService::removeUserFromGroup(int64_t userId, int64_t groupId) {
        auto wuow = m_database.createWriteUnitOfWork();

        auto removeResult = m_userGroupRepo.removeUserFromGroup(wuow, UserGroup{.userId = userId,
                                                                                .groupId = groupId});

        if (!removeResult) {
            return std::unexpected(mapPersistenceError(removeResult.error()));
        }

        wuow.commit();
        return {};
    }

    ServiceResult<Group> GroupService::getGroupById(int64_t groupId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto result = m_groupRepo.getById(ruow, groupId);
        if (!result) {
            return std::unexpected(mapPersistenceError(result.error()));
        }

        return *result;
    }

    ServiceResult<std::vector<Group>> GroupService::getAllGroups() {
        auto ruow = m_database.createReadUnitOfWork();

        auto result = m_groupRepo.getAll(ruow);
        if (!result) {
            return std::unexpected(mapPersistenceError(result.error()));
        }

        return *result;
    }

    ServiceResult<std::vector<int64_t>> GroupService::getUserGroups(int64_t userId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto groupsResult = m_userGroupRepo.getGroupIdsOfUser(ruow, userId);
        if (!groupsResult) {
            return std::unexpected(mapPersistenceError(groupsResult.error()));
        }

        return *groupsResult;
    }

    ServiceResult<std::vector<int64_t>> GroupService::getGroupUsers(int64_t groupId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto usersResult = m_userGroupRepo.getUserIdsOfGroup(ruow, groupId);
        if (!usersResult) {
            return std::unexpected(mapPersistenceError(usersResult.error()));
        }

        return *usersResult;
    }

}