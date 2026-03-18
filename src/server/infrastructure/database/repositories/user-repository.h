#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/models/user.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class UserRepository {

            SqliteDatabase& m_db;

        public:

            explicit UserRepository(SqliteDatabase& db)
                : m_db(db) {}


            auto getById(int64_t id) {
                return m_db.submitRead([id](auto& storage) {
                    return storage.template get<User>(id);
                });
            }

            auto getByLogin(const std::string& login) {
                return m_db.submitRead([login](auto& storage) -> std::optional<User> {
                    auto all = storage.template get_all<User>(sqlite_orm::where(sqlite_orm::c(&User::login) == login));

                    if (all.empty()) {
                        return std::nullopt;
                    }

                    return all.front();
                });
            }

            auto getAll() {
                return m_db.submitRead([](auto& storage) {
                    return storage.template get_all<User>();
                });
            }

            auto create(User user) {
                return m_db.submitWrite([user = std::move(user)](auto& storage) mutable {
                    return storage.insert(user);
                });
            }

            auto update(User user) {
                return m_db.submitWrite([user = std::move(user)](auto& storage) mutable {
                    storage.update(user);
                });
            }

            auto remove(int64_t id) {
                return m_db.submitWrite([id](auto& storage) mutable {
                    storage.template remove<User>(id);
                });
            }

    };

}