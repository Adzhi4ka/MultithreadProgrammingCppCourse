#pragma once

#include "schema.h"

#include <deque>

namespace infrastructure::db::sqlite {
   
    class DatabaseFactory {

            std::string m_dbPath;
            size_t m_readersCount;

        public:

            using Storage = decltype(infrastructure::db::sqlite::makeStorage({}));

            DatabaseFactory(std::string dbPath, size_t readersCount) : m_dbPath(std::move(dbPath)), m_readersCount(readersCount) {}

            Storage createWriter() const {
                auto s = infrastructure::db::sqlite::makeStorage(m_dbPath);
                s.sync_schema();
                return s;
            }

            std::deque<Storage> createReaders() const {
                std::deque<Storage> readers;
                for(size_t i = 0; i < m_readersCount; ++i) {
                    readers.emplace_back(infrastructure::db::sqlite::makeStorage(m_dbPath));
                }
                return readers;
            }

    };

}