#include "infrastructure/database/sqlite/sqlite-database.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <gtest/gtest.h>

#include <atomic>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>

#include "infrastructure/database/sqlite/database-factory.h"

namespace infrastructure::db::sqlite::tests {

using namespace std::chrono_literals;

class SqliteDatabaseTest : public ::testing::Test {

    protected:

        std::string m_dbPath{"sqlite_database_test.db"};
        std::unique_ptr<DatabaseFactory> m_factory;
        std::unique_ptr<SqliteDatabase> m_database;

    protected:

        void SetUp() override {
            std::filesystem::remove(m_dbPath);

            {
                SQLite::Database db(m_dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

                db.exec(
                    "CREATE TABLE IF NOT EXISTS test_data ("
                    "id INTEGER PRIMARY KEY, "
                    "value TEXT NOT NULL"
                    ");");
            }

            m_factory = std::make_unique<DatabaseFactory>(m_dbPath, 2);
            m_database = std::make_unique<SqliteDatabase>(*m_factory);
        }

        void TearDown() override {
            m_database.reset();
            m_factory.reset();
            std::filesystem::remove(m_dbPath);
        }

        static void insertRow(SQLite::Database& db, int id, const std::string& value) {
            SQLite::Statement statement(db, "INSERT INTO test_data (id, value) VALUES (?, ?);");

            statement.bind(1, id);
            statement.bind(2, value);

            statement.exec();
        }

        static int countRows(SQLite::Database& db) {
            SQLite::Statement statement(db, "SELECT COUNT(*) FROM test_data;");

            statement.executeStep();

            return statement.getColumn(0).getInt();
        }

        static std::string getValueById(SQLite::Database& db, int id) {
            SQLite::Statement statement(db, "SELECT value FROM test_data WHERE id = ?;");

            statement.bind(1, id);

            if (!statement.executeStep()) {
                return {};
            }

            return statement.getColumn(0).getString();
        }
};

TEST_F(SqliteDatabaseTest, WriteUnitOfWorkCommitPersistsChanges) {
    auto wuow = m_database->createWriteUnitOfWork();

    insertRow(wuow.connection(), 1, "committed");
    wuow.commit();

    SQLite::Database verifyDb(m_dbPath, SQLite::OPEN_READONLY);

    ASSERT_EQ(countRows(verifyDb), 1);
    ASSERT_EQ(getValueById(verifyDb, 1), "committed");
}

TEST_F(SqliteDatabaseTest, WriteUnitOfWorkRollbackCancelsChanges) {
    auto wuow = m_database->createWriteUnitOfWork();

    insertRow(wuow.connection(), 1, "rolled_back");
    wuow.rollback();

    SQLite::Database verifyDb(m_dbPath, SQLite::OPEN_READONLY);

    ASSERT_EQ(countRows(verifyDb), 0);
}

TEST_F(SqliteDatabaseTest, DestructorRollsBackUnfinishedWriteTransaction) {
    {
        auto wuow = m_database->createWriteUnitOfWork();
        insertRow(wuow.connection(), 1, "not_committed");
    }

    SQLite::Database verifyDb(m_dbPath, SQLite::OPEN_READONLY);

    ASSERT_EQ(countRows(verifyDb), 0);
}

TEST_F(SqliteDatabaseTest, ReadUnitOfWorkReadsCommittedData) {
    {
        auto wuow = m_database->createWriteUnitOfWork();
        insertRow(wuow.connection(), 1, "hello");
        wuow.commit();
    }

    auto ruow = m_database->createReadUnitOfWork();

    SQLite::Statement statement(ruow.connection(), "SELECT value FROM test_data WHERE id = ?;");

    statement.bind(1, 1);

    ASSERT_TRUE(statement.executeStep());
    ASSERT_EQ(statement.getColumn(0).getString(), "hello");
}

TEST_F(SqliteDatabaseTest, ReadUnitOfWorkReadsUncommittedData) {
    auto wuow = m_database->createWriteUnitOfWork();
    insertRow(wuow.connection(), 1, "hello");

    auto ruow = m_database->createReadUnitOfWork();

    SQLite::Statement statement(ruow.connection(), "SELECT value FROM test_data WHERE id = ?;");

    statement.bind(1, 1);

    ASSERT_FALSE(statement.executeStep());
}

TEST_F(SqliteDatabaseTest, SecondWriterWaitsUntilFirstWriterReleased) {
    std::atomic<bool> secondWriterAcquired{false};

    auto firstWuow = m_database->createWriteUnitOfWork();

    std::thread secondWriterThread([&]() {
        auto secondWuow = m_database->createWriteUnitOfWork();
        secondWriterAcquired.store(true);
        secondWuow.rollback();
    });

    std::this_thread::sleep_for(150ms);
    ASSERT_FALSE(secondWriterAcquired.load());

    firstWuow.rollback();

    secondWriterThread.join();

    ASSERT_TRUE(secondWriterAcquired.load());
}

TEST_F(SqliteDatabaseTest, WriteUnitOfWorkCanBeAcquiredAgainAfterRollback) {
    {
        auto wuow = m_database->createWriteUnitOfWork();
        insertRow(wuow.connection(), 1, "temp");
        wuow.rollback();
    }

    {
        auto wuow = m_database->createWriteUnitOfWork();
        insertRow(wuow.connection(), 2, "persisted");
        wuow.commit();
    }

    SQLite::Database verifyDb(m_dbPath, SQLite::OPEN_READONLY);

    ASSERT_EQ(countRows(verifyDb), 1);
    ASSERT_EQ(getValueById(verifyDb, 2), "persisted");
}

TEST_F(SqliteDatabaseTest, TwoReadersCanExistAtTheSameTime) {
    auto firstReader = m_database->createReadUnitOfWork();
    auto secondReader = m_database->createReadUnitOfWork();

    SQLite::Statement firstStatement(firstReader.connection(), "SELECT COUNT(*) FROM test_data;");

    SQLite::Statement secondStatement(secondReader.connection(), "SELECT COUNT(*) FROM test_data;");

    ASSERT_TRUE(firstStatement.executeStep());
    ASSERT_TRUE(secondStatement.executeStep());

    ASSERT_EQ(firstStatement.getColumn(0).getInt(), 0);
    ASSERT_EQ(secondStatement.getColumn(0).getInt(), 0);
}

TEST_F(SqliteDatabaseTest, ReaderIsReturnedToPoolAfterDestruction) {
    std::atomic<bool> thirdReaderAcquired{false};

    auto first = m_database->createReadUnitOfWork();
    auto second = m_database->createReadUnitOfWork();

    std::thread thirdReaderThread([&]() {
        auto secondWuow = m_database->createReadUnitOfWork();
        thirdReaderAcquired.store(true);
        secondWuow.close();
    });

    std::this_thread::sleep_for(150ms);
    ASSERT_FALSE(thirdReaderAcquired.load());

    first.close();
    second.close();

    thirdReaderThread.join();

    ASSERT_TRUE(thirdReaderAcquired.load());
}

}  // namespace infrastructure::db::sqlite::tests