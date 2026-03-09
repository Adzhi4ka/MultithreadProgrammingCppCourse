#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/models/file-lock.h"

#include <cstdint>
#include <utility>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class FileLockRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileLockRepository(SqliteDatabase& db)
                : m_db(db) {}


            void get(int64_t id, auto&& callback) {
                m_db.submitRead([id, cb = std::forward<decltype(callback)>(callback)](auto& storage) {
                    auto fileLock = storage.template get<FileLock>(id);
                    cb(fileLock);
                });
            }

            void getAll(auto&& callback) {
                m_db.submitRead([cb = std::forward<decltype(callback)>(callback)](auto& storage) {
                    auto fileLocks = storage.template get_all<FileLock>();
                    cb(fileLocks);
                });
            }

            void add(FileLock fileLock, auto&& callback) {
                m_db.submitWrite([fileLock = std::move(fileLock), cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.insert(fileLock);
                    cb();
                });
            }

            void update(FileLock fileLock, auto&& callback) {
                m_db.submitWrite([fileLock = std::move(fileLock), cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.update(fileLock);
                    cb();
                });
            }

            void remove(int64_t id, auto&& callback) {
                m_db.submitWrite([id, cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.template remove<FileLock>(id);
                    cb();
                });
            }

        };

}