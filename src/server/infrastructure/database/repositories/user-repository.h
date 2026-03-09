#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/models/user.h"

#include <cstdint>
#include <utility>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class UserRepository {

            SqliteDatabase& m_db;

        public:

            explicit UserRepository(SqliteDatabase& db)
                : m_db(db) {}


            void get(int64_t id, auto&& callback) {
                m_db.submitRead([id, cb = std::forward<decltype(callback)>(callback)](auto& storage) {
                    auto user = storage.template get<User>(id);
                    cb(user);
                });
            }

            void getAll(auto&& callback) {
                m_db.submitRead([cb = std::forward<decltype(callback)>(callback)](auto& storage) {
                    auto users = storage.template get_all<User>();
                    cb(users);
                });
            }

            void add(User user, auto&& callback) {
                m_db.submitWrite([user = std::move(user), cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.insert(user);
                    cb();
                });
            }

            void update(User user, auto&& callback) {
                m_db.submitWrite([user = std::move(user), cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.update(user);
                    cb();
                });
            }

            void remove(int64_t id, auto&& callback) {
                m_db.submitWrite([id, cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.template remove<User>(id);
                    cb();
                });
            }

        };

}