#include "schema.h"

#include "migrations/migration_init.h"

namespace infrastructure::db::sqlite {

void applySchema(SQLite::Database& db) {

    db.exec("PRAGMA foreign_keys = ON;");
    db.exec("PRAGMA journal_mode = WAL;");

    for (const auto& createTable : migrations::kCreateTablesTopSortedArray) {
        db.exec(createTable);
    }
}

}  // namespace infrastructure::db::sqlite