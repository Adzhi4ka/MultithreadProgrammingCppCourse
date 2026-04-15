#include "user-group-repository.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    void UserGroupRepository::addUserToGroup(WriteUnitOfWork& wuov, UserGroup userGroup) {
        constexpr const char* const sql = {
            "INSERT INTO user_group "
                "(user_id, group_id) "
            "VALUES (?, ?);"
        };

        SQLite::Statement statement(wuov.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, userGroup.userId);
            statement.bind(bindIndex++, userGroup.userId);
        }

        statement.executeStep();
    }

    void UserGroupRepository::removeUserFromGroup(WriteUnitOfWork& wuov, UserGroup userGroup) {
        constexpr const char* const sql = {
            "DELETE FROM user_group "
            "WHERE user_id = ? AND group_id = ?;"
        };

        SQLite::Statement statement(wuov.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, userGroup.userId);
            statement.bind(bindIndex++, userGroup.userId);
        }

        statement.executeStep();
    }

    std::vector<int64_t> UserGroupRepository::getGroupIdsOfUser(UnitOfWork& uow, int64_t userId) {
        constexpr const char* const sql = {
            "SELECT group_id "
            "FROM user_group "
            "WHERE user_id = ?;"
        };

        SQLite::Statement statement(uow.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, userId);
        }

        std::vector<int64_t> groups;

        while (statement.executeStep()) {
            groups.emplace_back(statement.getColumn(0).getInt64());
        }

        return groups;
    }

    std::vector<int64_t> UserGroupRepository::getUserIdsOfGroup(UnitOfWork& uow, int64_t groupId) {
        constexpr const char* const sql = {
            "SELECT user_id "
            "FROM user_group "
            "WHERE group_id = ?;"
        };

        SQLite::Statement statement(uow.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, groupId);
        }

        std::vector<int64_t> users;

        while (statement.executeStep()) {
            users.emplace_back(statement.getColumn(0).getInt64());
        }

        return users;
    }


}