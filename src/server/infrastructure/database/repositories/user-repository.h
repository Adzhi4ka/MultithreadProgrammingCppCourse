#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/user.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class UserRepository {

            SqliteDatabase& m_db;

        public:

            explicit UserRepository(SqliteDatabase& db) : m_db(db) {}

            void create(WriteUnitOfWork& wuov, User user);
            void update(WriteUnitOfWork& wuov, User user);
            void remove(WriteUnitOfWork& wuov, int64_t id);

            User getById(UnitOfWork& uov, int64_t id);
            User getByLogin(UnitOfWork& uov, const std::string& login);
            std::vector<User> getAll(UnitOfWork& uov);
    };

}