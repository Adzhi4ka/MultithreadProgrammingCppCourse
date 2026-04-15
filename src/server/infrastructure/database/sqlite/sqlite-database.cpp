#include "sqlite-database.h"

namespace infrastructure::db::sqlite {

    UnitOfWork::UnitOfWork(SQLite::Database& db) noexcept : m_db(db) {}

    SQLite::Database& UnitOfWork::connection() noexcept {
        return m_db;
    }



    ReadUnitOfWork::ReadUnitOfWork(SqliteDatabase& owner, SQLite::Database& db) : UnitOfWork(db), m_pOwner(&owner) {}

    ReadUnitOfWork::~ReadUnitOfWork() {
        close();
    };

    void ReadUnitOfWork::close() {
        if (!m_pOwner) return;

        m_pOwner->releaseReader(m_db);
        m_pOwner = nullptr;
    }



    WriteUnitOfWork::WriteUnitOfWork(SqliteDatabase& owner, SQLite::Database& db) : UnitOfWork(db), m_pOwner(&owner) {
        m_db.exec(kBegin);
    }

    WriteUnitOfWork::~WriteUnitOfWork() {
        rollback();
        close();
    }

    void WriteUnitOfWork::commit() {
        if (m_finished) return;

        m_db.exec(kCommit);
        m_finished = true;

        close();
    }

    void WriteUnitOfWork::rollback() noexcept {
        if (m_finished) return;

        try {
            m_db.exec(kRollback);
        } catch (...) {}

        m_finished = true;

        close();
    }

    void WriteUnitOfWork::close() {
        if (!m_pOwner) return;

        m_pOwner->releaseWriter();
        m_pOwner = nullptr;
    }



    SqliteDatabase::SqliteDatabase(DatabaseFactory& factory) : m_writerCon(factory.createWriter()),
                                                               m_writerBinSem(1),
                                                               m_readerCons(factory.createReaders()),
                                                               m_readersSemaphore(m_readerCons.size()) {
        for (auto& databaseUniquePtr : m_readerCons) {
            m_availableReaders.push(databaseUniquePtr.get());
        }
    }

    WriteUnitOfWork SqliteDatabase::createWriteUnitOfWork() {
        m_writerBinSem.acquire();

        try {
            return WriteUnitOfWork(*this, *m_writerCon);
        } catch (...) {
            releaseWriter();
            throw;
        }
    }

    ReadUnitOfWork SqliteDatabase::createReadUnitOfWork() {

        m_readersSemaphore.acquire();

        Database* reader;

        {
            std::lock_guard lock(m_readersMutex);
            reader = m_availableReaders.front();
            m_availableReaders.pop();
        }

        try {
            return ReadUnitOfWork(*this, *reader);
        } catch (...) {
            releaseReader(*reader);
            throw;
        }

    }

    void SqliteDatabase::releaseWriter() {
        m_writerBinSem.release();
    }

    void SqliteDatabase::releaseReader(Database& dbReader) {
        {
            std::lock_guard lock(m_readersMutex);
            m_availableReaders.push(&dbReader);
        }

        m_readersSemaphore.release();
    }

} // namespace infrastructure::db::sqlite