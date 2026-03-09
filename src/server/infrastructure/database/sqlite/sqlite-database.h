#pragma once

#include "database-factory.h"

#include <deque>
#include <mutex>

#include <boost/asio.hpp>

#include <sqlite_orm/sqlite_orm.h>

namespace infrastructure::db::sqlite {

    class SqliteDatabase {

            using Storage = typename DatabaseFactory::Storage;

            Storage m_writer;

            std::deque<Storage> m_readers;

            using ThreadPool = boost::asio::thread_pool;
            using WriterStrand = boost::asio::strand<boost::asio::thread_pool::executor_type>;
            ThreadPool& m_rThreadPool;
            WriterStrand m_writerStrand;

            mutable std::mutex m_readerChooseMutex;
            size_t m_nextReader;

        public:

            explicit SqliteDatabase(boost::asio::thread_pool& threadPool, DatabaseFactory factory)
                : m_writer{factory.createWriter()},
                  m_readers{factory.createReaders()},
                  m_rThreadPool(threadPool),
                  m_writerStrand(threadPool.executor()),
                  m_nextReader{0} {}

            template<typename Func>
            void submitWrite(Func&& func) {
                boost::asio::post(m_writerStrand,
                    [this, func = std::forward<Func>(func)]() mutable {
                        func(m_writer);
                    });
            }

            template<typename Func>
            void submitRead(Func&& func) {
                boost::asio::post(m_rThreadPool.executor(),
                    [this, func = std::forward<Func>(func)]() mutable {
                        // func(chooseReader());
                    });
            }

        private:

            decltype(auto) chooseReader() {
                std::lock_guard lg(m_readerChooseMutex);

                auto& reader = m_readers[m_nextReader];
                m_nextReader = (m_nextReader + 1) % m_readers.size();

                return reader;
            }

        public:
            
            class TransactionScope {

                    std::unique_lock<std::mutex> m_lock;
                    decltype(std::declval<decltype(SqliteDatabase::m_writer)>().transaction_guard()) m_guard;

                public:

                    TransactionScope(SqliteDatabase& storage, std::mutex& mutex)
                        : m_lock(mutex),
                          m_guard(storage.m_writer.transaction_guard())
                    {}

                    void commit() noexcept {
                        m_guard.commit();
                    }

            };

    };

}