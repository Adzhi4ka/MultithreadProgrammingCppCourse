#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/models/group.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class GroupRepository {

            SqliteDatabase& m_db;

        public:

            explicit GroupRepository(SqliteDatabase& db)
                : m_db(db) {}


            auto getById(int64_t id) {
                return m_db.submitRead([id](auto& storage) {
                    return storage.template get<Group>(id);
                });
            }

            auto getByName(const std::string& name) {
                return m_db.submitRead([name](auto& storage) -> std::optional<Group> {
                    auto all = storage.template  get_all<Group>(sqlite_orm::where(sqlite_orm::c(&Group::name) == name));
                    if (all.empty()) return std::nullopt;
                    return all.front();
                });
            }

            auto getAll() {
                return m_db.submitRead([](auto& storage) {
                    return storage.template get_all<Group>();
                });
            }

            auto create(Group group) {
                return m_db.submitWrite([group = std::move(group)](auto& storage) mutable {
                    return storage.insert(group);
                });
            }

            auto remove(int64_t id) {
                return m_db.submitWrite([id](auto& storage) mutable {
                    storage.template remove<Group>(id);
                });
            }

    };

}