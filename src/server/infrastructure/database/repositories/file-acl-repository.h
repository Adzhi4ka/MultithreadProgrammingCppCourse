#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/models/file-acl.h"

#include <cstdint>
#include <utility>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class FileAclRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileAclRepository(SqliteDatabase& db)
                : m_db(db) {}


            void get(int64_t id, auto&& callback) {
                m_db.submitRead([id, cb = std::forward<decltype(callback)>(callback)](auto& storage) {
                    auto fileAcl = storage.template get<FileAcl>(id);
                    cb(fileAcl);
                });
            }

            void getAll(auto&& callback) {
                m_db.submitRead([cb = std::forward<decltype(callback)>(callback)](auto& storage) {
                    auto fileAcls = storage.template get_all<FileAcl>();
                    cb(fileAcls);
                });
            }

            void add(FileAcl fileAcl, auto&& callback) {
                m_db.submitWrite([fileAcl = std::move(fileAcl), cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.insert(fileAcl);
                    cb();
                });
            }

            void update(FileAcl fileAcl, auto&& callback) {
                m_db.submitWrite([fileAcl = std::move(fileAcl), cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.update(fileAcl);
                    cb();
                });
            }

            void remove(int64_t id, auto&& callback) {
                m_db.submitWrite([id, cb = std::forward<decltype(callback)>(callback)](auto& storage) mutable {
                    storage.template remove<FileAcl>(id);
                    cb();
                });
            }

        };

}