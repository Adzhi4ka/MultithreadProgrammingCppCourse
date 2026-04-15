#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "domain/models/file.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class FileRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileRepository(SqliteDatabase& db)
                : m_db(db) {}

            void create(WriteUnitOfWork& wuov, File file);
            void update(WriteUnitOfWork& wuov, File file);
            void remove(WriteUnitOfWork& wuov, int64_t id);

            File getById(UnitOfWork& uow, int64_t id);
            std::vector<File> getAll(UnitOfWork& uow);

    };

}