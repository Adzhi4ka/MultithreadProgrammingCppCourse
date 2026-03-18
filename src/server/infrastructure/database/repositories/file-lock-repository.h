#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/models/file-lock.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class FileLockRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileLockRepository(SqliteDatabase& db)
                : m_db(db) {}


            auto lock(int64_t fileId, int64_t userId, int64_t leaseUntil) {
                return m_db.submitWrite([fileId, userId, leaseUntil](auto& storage) mutable {
                    
                });
            }

            auto unlock(int64_t fileId) {
                return m_db.submitWrite([fileId](auto& storage) mutable {
                    storage.template remove<FileLock>(fileId);
                });
            }

            auto getLock(int64_t fileId) {
                return m_db.submitRead([fileId](auto& storage) -> std::optional<FileLock> {
                    auto all = storage.template get_all<FileLock>(where(c(&FileLock::fileId) == fileId));
                    if (all.empty()) return std::nullopt;
                    return all.front();
                });
            }

            auto updateLease(int64_t fileId, int64_t leaseUntil) {
                return m_db.submitWrite([fileId, leaseUntil](auto& storage) mutable {
                    storage.update_column<FileLock>(&FileLock::leaseUntil, leaseUntil, where(c(&FileLock::fileId) == fileId));
                });
            }

    };

}