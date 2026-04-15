#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "domain/models/group.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class GroupRepository {

            SqliteDatabase& m_db;

        public:

            explicit GroupRepository(SqliteDatabase& db)
                : m_db(db) {}

            void create(WriteUnitOfWork& wuov, Group group);
            void remove(WriteUnitOfWork& wuov, int64_t id);

            Group getById(UnitOfWork& uov, int64_t id);
            Group getByName(UnitOfWork& uov, const std::string& name);
            std::vector<Group> getAll(UnitOfWork& uov);

    };

}