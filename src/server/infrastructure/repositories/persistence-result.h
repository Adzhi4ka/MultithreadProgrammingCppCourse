#pragma once

#include <SQLiteCpp/Exception.h>
#include <sqlite3.h>

#include <cstdint>
#include <expected>

namespace infrastructure::repositories {

enum class PersistenceError : uint8_t { NotFound, Conflict, InternalError };

template <typename T>
using PersistenceResult = std::expected<T, PersistenceError>;

inline PersistenceError mapSqliteException(const SQLite::Exception& ex) noexcept {
    const int code = ex.getExtendedErrorCode();

    switch (code) {
        case SQLITE_CONSTRAINT:
        case SQLITE_CONSTRAINT_CHECK:
        case SQLITE_CONSTRAINT_FOREIGNKEY:
        case SQLITE_CONSTRAINT_NOTNULL:
        case SQLITE_CONSTRAINT_PRIMARYKEY:
        case SQLITE_CONSTRAINT_UNIQUE:
        case SQLITE_CONSTRAINT_ROWID:
        case SQLITE_CONSTRAINT_DATATYPE:
            return PersistenceError::Conflict;

        default:
            return PersistenceError::InternalError;
    }
}

}  // namespace infrastructure::repositories