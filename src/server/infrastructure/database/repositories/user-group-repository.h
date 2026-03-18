#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class UserGroupRepository {

            SqliteDatabase& m_db;

        public:

            explicit UserGroupRepository(SqliteDatabase& db) : m_db(db) {}

            auto addUserToGroup(int64_t userId, int64_t groupId) {
                return m_db.submitWrite([userId, groupId](auto& storage) mutable {
                    storage.insert(UserGroup{userId, groupId});
                });
            }

            auto removeUserFromGroup(int64_t userId, int64_t groupId) {
                return m_db.submitWrite([userId, groupId](auto& storage) mutable {
                    storage.template remove<UserGroup>(std::make_tuple(userId, groupId));
                });
            }

            auto getGroupsOfUser(int64_t userId) {
                return m_db.submitRead([userId](auto& storage) {
                    auto rows = storage.template get_all<UserGroup>(sqlite_orm::where(sqlite_orm::c(&UserGroup::userId) == userId));

                    std::vector<int64_t> result;
                    for (auto& r : rows) {
                        result.emplace_back(std::move(r.groupId));
                    }

                    return result;
                });
            }

            auto getUsersOfGroup(int64_t groupId) {
                return m_db.submitRead([groupId](auto& storage) {
                    auto rows = storage.template get_all<UserGroup>(sqlite_orm::where(sqlite_orm::c(&UserGroup::groupId) == groupId));

                    std::vector<int64_t> result;
                    for (auto& r : rows) {
                        result.emplace_back(std::move(r.userId));
                    }

                    return result;
                });
            }

    };

}