#include "database-factory.h"

#include "schema.h"
#include <memory>

namespace infrastructure::db::sqlite {

    DatabaseFactory::DatabaseUniquePtr DatabaseFactory::createWriter() const {
        auto pDb = std::make_unique<SQLite::Database>(m_dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        applySchema(*pDb);

        return pDb;
    }

    std::vector<DatabaseFactory::DatabaseUniquePtr> DatabaseFactory::createReaders() const {
        std::vector<DatabaseFactory::DatabaseUniquePtr> readers;

        for(size_t i = 0; i < m_readersCount; ++i) {
            readers.emplace_back(std::make_unique<SQLite::Database>(m_dbPath, SQLite::OPEN_READONLY));
        }

        return readers;
    }

} // namespace infrastructure::db::sqlite