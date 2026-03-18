#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/models/file.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class FileRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileRepository(SqliteDatabase& db)
                : m_db(db) {}


            auto getById(int64_t id) {
                return m_db.submitRead([id](auto& storage) {
                    return storage.template get<File>(id);
                });
            }

            auto getAll() {
                return m_db.submitRead([](auto& storage) {
                    return storage.template get_all<File>();
                });
            }

            auto create(File file) {
                return m_db.submitWrite([file = std::move(file)](auto& storage) mutable {
                    return storage.insert(file);
                });
            }

            auto update(File file) {
                return m_db.submitWrite([file = std::move(file)](auto& storage) mutable {
                    storage.update(file);
                });
            }

            auto remove(int64_t id) {
                return m_db.submitWrite([id](auto& storage) mutable {
                    storage.template remove<File>(id);
                });
            }

    };

}