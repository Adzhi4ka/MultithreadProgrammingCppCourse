#pragma once

#include "database-factory.h"

#include <mutex>
#include <semaphore>
#include <vector>
#include <queue>

#include <SQLiteCpp/SQLiteCpp.h>

namespace infrastructure::db::sqlite {

    class SqliteDatabase;

    // по факту можно на статичный полиморфизм переделать, но пока это в дальний ящик
    class UnitOfWork {

        protected:

            SQLite::Database& m_db;

        public:

            explicit UnitOfWork(SQLite::Database& db) noexcept;

            virtual ~UnitOfWork() = default;

            SQLite::Database& connection() noexcept;

    };

    class ReadUnitOfWork final : public UnitOfWork {

            SqliteDatabase* m_pOwner;

        public:

            explicit ReadUnitOfWork(SqliteDatabase& owner, SQLite::Database& db);

            ~ReadUnitOfWork() override;

            void close();

    };

    class WriteUnitOfWork final : public UnitOfWork {

            static constexpr const char* const kBegin = "BEGIN IMMEDIATE";
            static constexpr const char* const kCommit = "COMMIT";
            static constexpr const char* const kRollback = "ROLLBACK";

            SqliteDatabase* m_pOwner;

            bool m_finished {false};

        public:

            explicit WriteUnitOfWork(SqliteDatabase& owner, SQLite::Database& db);

            ~WriteUnitOfWork() override;

        public:

            void commit();
            void rollback() noexcept;
            void close();

    };

    class SqliteDatabase {

            using Database = SQLite::Database;
            using DatabaseUniquePtr = std::unique_ptr<Database>;

            DatabaseUniquePtr m_writerCon;
            std::binary_semaphore m_writerBinSem;

            std::vector<DatabaseUniquePtr> m_readerCons;
            std::queue<Database*> m_availableReaders;
            std::mutex m_readersMutex;
            std::counting_semaphore<> m_readersSemaphore;

        public:

            explicit SqliteDatabase(DatabaseFactory& factory);

            WriteUnitOfWork createWriteUnitOfWork();
            ReadUnitOfWork createReadUnitOfWork();

        private:

            friend class ReadUnitOfWork;
            friend class WriteUnitOfWork;

            void releaseWriter();
            void releaseReader(Database& dbReader);

    };

} // namespace infrastructure::db::sqlite