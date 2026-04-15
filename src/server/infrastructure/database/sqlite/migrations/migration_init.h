#pragma once

namespace infrastructure::db::sqlite::migrations {

    constexpr const char* const kCreateTableUsers = {
        "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY, "
            "login TEXT NOT NULL UNIQUE, "
            "password_hash TEXT NOT NULL"
        ");"
    };

    constexpr const char* const kCreateTableGroups = {
        "CREATE TABLE IF NOT EXISTS groups ("
            "id INTEGER PRIMARY KEY, "
            "name TEXT NOT NULL UNIQUE "
        ");"
    };

    constexpr const char* const kCreateTableUserGroups = {
        "CREATE TABLE IF NOT EXISTS user_group ("
            "user_id INTEGER NOT NULL, "
            "group_id INTEGER NOT NULL, "
            "PRIMARY KEY (user_id, group_id), "
            "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE, "
            "FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE "
        ");"
    };

    constexpr const char* const kCreateTableFiles = {
        "CREATE TABLE IF NOT EXISTS files ("
            "id INTEGER PRIMARY KEY, "
            "full_logical_name TEXT NOT NULL UNIQUE, "
            "current_version_id INTEGER NOT NULL, "
            "max_version_count INTEGER NOT NULL, "
            "created_at INTEGER NOT NULL, "
            "created_by INTEGER NOT NULL, "
            "FOREIGN KEY (created_by) REFERENCES users(id) "
        ");"
    };

    constexpr const char* const kCreateTableFileVersions = {
        "CREATE TABLE IF NOT EXISTS file_versions ("
            "id INTEGER PRIMARY KEY, "
            "file_id INTEGER NOT NULL, "
            "version INTEGER NOT NULL, "
            "logical_name_snapshot TEXT NOT NULL, "
            "physical_path_name INTEGER NOT NULL, "
            "created_at INTEGER NOT NULL, "
            "FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE, "
            "UNIQUE(file_id, version) "
        ");"
    };

    constexpr const char* const kCreateTableFileAcl = {
        "CREATE TABLE IF NOT EXISTS file_acl ("
            "file_id INTEGER NOT NULL, "
            "group_id INTEGER NOT NULL, "
            "acl_level INTEGER NOT NULL, "
            "PRIMARY KEY (file_id, group_id), "
            "FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE, "
            "FOREIGN KEY (group_id) REFERENCES groups(id) "
        ");"
    };

    constexpr const char* const kCreateTableFileLock = {
        "CREATE TABLE IF NOT EXISTS file_lock ("
            "file_id INTEGER PRIMARY KEY, "
            "user_id INTEGER NOT NULL, "
            "lease_until INTEGER NOT NULL, "
            "FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE, "
            "FOREIGN KEY (user_id) REFERENCES users(id) "
        ");"
    };

    constexpr const char* const kCreateTablesTopSortedArray[] = {
        kCreateTableUsers,
        kCreateTableGroups,
        kCreateTableUserGroups,
        kCreateTableFiles,
        kCreateTableFileVersions,
        kCreateTableFileAcl,
        kCreateTableFileLock
    };

} // namespace infrastructure::db::sqlite::migrations