#include "sqlite-database.h"

namespace infrastructure::db::sqlite {

    UnitOfWork::UnitOfWork(SQLite::Database& db) noexcept : m_db(db) {}

    SQLite::Database& UnitOfWork::connection() noexcept {
        return m_db;
    }



    ReadUnitOfWork::ReadUnitOfWork(SqliteDatabase& owner, SQLite::Database& db) : UnitOfWork(db), m_owner(owner) {}

    ReadUnitOfWork::~ReadUnitOfWork() {
        release();
    };

    void ReadUnitOfWork::release() {
        m_owner.releaseReader(m_db);
    }



    WriteUnitOfWork::WriteUnitOfWork(SqliteDatabase& owner, SQLite::Database& db) : UnitOfWork(db), m_owner(owner) {
        m_db.exec(kBegin);
    }

    WriteUnitOfWork::~WriteUnitOfWork() {
        rollback();
        release();
    }
    void WriteUnitOfWork::commit() {
        if (m_finished) return;

        m_db.exec(kCommit);
        m_finished = true;
    }

    void WriteUnitOfWork::rollback() noexcept {
        if (m_finished) return;

        try {
            m_db.exec(kRollback);
        } catch (...) {}

        m_finished = true;
    }

    void WriteUnitOfWork::release() {
        m_owner.releaseWriter();
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