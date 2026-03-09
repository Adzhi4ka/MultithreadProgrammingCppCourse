#pragma once

#include "sqlite-database.h"

namespace infrastructure::db::sqlite {
   
   class TransactionFactory {

            using Database = infrastructure::db::sqlite::SqliteDatabase;

            Database& m_db;

            std::mutex m_transactionMutex;
        
        public:

            TransactionFactory(Database& db) : m_db(db) {}


            decltype(auto) operator()() {
                return Database::TransactionScope(m_db, m_transactionMutex);
            }

    };

}