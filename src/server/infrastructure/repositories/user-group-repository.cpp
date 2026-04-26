#include "user-group-repository.h"

namespace infrastructure::repositories {

    PersistenceResult<void> UserGroupRepository::addUserToGroup(WriteUnitOfWork& wuow, UserGroup userGroup) {
        constexpr const char* const sql = {
            "INSERT INTO user_group "
                "(user_id, group_id) "
            "VALUES (?, ?);"
        };

        try {
            SQLite::Statement statement(wuow.connection(), sql);

            {
                int bindIndex = 1;
                statement.bind(bindIndex++, userGroup.userId);
                statement.bind(bindIndex++, userGroup.groupId);
            }

            statement.exec();
            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<void> UserGroupRepository::removeUserFromGroup(WriteUnitOfWork& wuow, UserGroup userGroup) {
        constexpr const char* const sql = {
            "DELETE FROM user_group "
            "WHERE user_id = ? AND group_id = ?;"
        };

        try {
            SQLite::Statement statement(wuow.connection(), sql);

            {
                int bindIndex = 1;
                statement.bind(bindIndex++, userGroup.userId);
                statement.bind(bindIndex++, userGroup.groupId);
            }

            statement.exec();

            if (wuow.connection().getChanges() == 0) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<std::vector<int64_t>> UserGroupRepository::getGroupIdsOfUser(UnitOfWork& uow, int64_t userId) {
        constexpr const char* const sql = {
            "SELECT group_id "
            "FROM user_group "
            "WHERE user_id = ?;"
        };

        try {
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
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<std::vector<int64_t>> UserGroupRepository::getUserIdsOfGroup(UnitOfWork& uow, int64_t groupId) {
        constexpr const char* const sql = {
            "SELECT user_id "
            "FROM user_group "
            "WHERE group_id = ?;"
        };

        try {
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
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

}