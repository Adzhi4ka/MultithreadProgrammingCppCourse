#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/models/file.h"

#include <cstdint>
#include <utility>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class FileRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileRepository(SqliteDatabase& db)
                : m_db(db) {}


            void get(int64_t id, auto&& callback) {
                m_db.submitRead([id, cb = std::forward<decltype(callback)>(callback)](auto& storage) {
                    auto file = storage.template get<File>(id);
                    cb(file);
                });
            }

            void getAll(auto&& callback) {
                m_db.submitRead([cb = std::forward<decltype(callback)>(callback)](auto& storage) {
                    auto files = storage.template get_all<File>();
                    cb(files);
                });
            }

            void add(File file, auto&& callback) {
                m_db.submitWrite([file = std::move(file), cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.insert(file);
                    cb();
                });
            }

            void update(File file, auto&& callback) {
                m_db.submitWrite([file = std::move(file), cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.update(file);
                    cb();
                });
            }

            void remove(int64_t id, auto&& callback) {
                m_db.submitWrite([id, cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.template remove<File>(id);
                    cb();
                });
            }

        };

}