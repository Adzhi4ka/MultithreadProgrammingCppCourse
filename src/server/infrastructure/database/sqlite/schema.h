#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

namespace infrastructure::db::sqlite {

    void applySchema(SQLite::Database& db);

} // namespace infrastructure::db::sqlite