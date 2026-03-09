#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/models/group.h"

#include <cstdint>
#include <utility>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class GroupRepository {

            SqliteDatabase& m_db;

        public:

            explicit GroupRepository(SqliteDatabase& db)
                : m_db(db) {}


            void get(int64_t id, auto&& callback) {
                m_db.submitRead([id, cb = std::forward<decltype(callback)>(callback)](auto& storage) {
                    auto group = storage.template get<Group>(id);
                    cb(group);
                });
            }

            void getAll(auto&& callback) {
                m_db.submitRead([cb = std::forward<decltype(callback)>(callback)](auto& storage) {
                    auto groups = storage.template get_all<Group>();
                    cb(groups);
                });
            }

            void add(Group group, auto&& callback) {
                m_db.submitWrite([group = std::move(group), cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.insert(group);
                    cb();
                });
            }

            void update(Group group, auto&& callback) {
                m_db.submitWrite([group = std::move(group), cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.update(group);
                    cb();
                });
            }

            void remove(int64_t id, auto&& callback) {
                m_db.submitWrite([id, cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.template remove<Group>(id);
                    cb();
                });
            }

        };

}