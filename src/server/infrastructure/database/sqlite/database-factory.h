#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

#include <string>
#include <vector>

namespace infrastructure::db::sqlite {
   
    class DatabaseFactory {

            using Database = SQLite::Database;
            using DatabaseUniquePtr = std::unique_ptr<Database>;

            std::string m_dbPath;
            size_t m_readersCount;

        public:

            DatabaseFactory(std::string dbPath, size_t readersCount) noexcept
                 : m_dbPath(std::move(dbPath)), m_readersCount(readersCount) {}

            DatabaseUniquePtr createWriter() const;

            std::vector<DatabaseUniquePtr> createReaders() const;

    };

} // namespace infrastructure::db::sqlite