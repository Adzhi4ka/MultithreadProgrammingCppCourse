#pragma once

#include "database-factory.h"

#include <deque>
#include <mutex>
#include <future>
#include <boost/asio.hpp>

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

            explicit SqliteDatabase(boost::asio::thread_pool& threadPool, DatabaseFactory& factory)
                : m_writer(factory.createWriter()),
                  m_readers{factory.createReaders()},
                  m_rThreadPool(threadPool),
                  m_writerStrand(threadPool.executor()),
                  m_nextReader{0} {}

            template<typename Func>
            auto submitWrite(Func&& func) -> std::future<decltype(func(m_writer))> {
                using RetType = decltype(func(m_writer));

                auto promise = std::make_shared<std::promise<RetType>>();
                auto fut = promise->get_future();

                boost::asio::post(m_writerStrand,
                    [this, func = std::forward<Func>(func), promise]() mutable {
                        try {
                            if constexpr (std::is_void_v<RetType>) {
                                func(m_writer);
                                promise->set_value();
                            } else {
                                promise->set_value(func(m_writer));
                            }
                        } catch (...) {
                            promise->set_exception(std::current_exception());
                        }
                    });

                return fut;
            }

            template<typename Func>
            auto submitRead(Func&& func) -> std::future<decltype(func(std::declval<Storage&>()))> {
                using RetType = decltype(func(std::declval<Storage&>()));

                auto promise = std::make_shared<std::promise<RetType>>();
                auto fut = promise->get_future();

                boost::asio::post(m_rThreadPool.executor(),
                    [this, func = std::forward<Func>(func), promise]() mutable {
                        try {
                            auto& reader = chooseReader();
                            if constexpr (std::is_void_v<RetType>) {
                                func(reader);
                                promise->set_value();
                            } else {
                                promise->set_value(func(reader));
                            }
                        } catch (...) {
                            promise->set_exception(std::current_exception());
                        }
                    });

                return fut;
            }

        private:

            Storage& chooseReader() {
                std::lock_guard lg(m_readerChooseMutex);

                auto& reader = m_readers[m_nextReader];
                m_nextReader = (m_nextReader + 1) % m_readers.size();

                return reader;
            }

        private:

            template<typename Func>
            auto submitTransaction(Func&& func) -> std::future<decltype(func(m_writer))> {
                using RetType = decltype(func(m_writer));

                auto promise = std::make_shared<std::promise<RetType>>();
                auto fut = promise->get_future();

                boost::asio::post(m_writerStrand,
                    [this, func = std::forward<Func>(func), promise]() mutable {
                        try {

                            auto transactionGuard = m_writer.transaction_guard();

                            if constexpr (std::is_void_v<RetType>) {
                                func(m_writer);
                                promise->set_value();
                            } else {
                                transactionGuard.commit();
                                promise->set_value(func(m_writer));
                            }

                            transactionGuard.commit();

                        } catch (...) {
                            promise->set_exception(std::current_exception());
                        }
                    });

                return fut;
            }

        public:
            
            class TransactionExecutor {

                SqliteDatabase& m_db;

            public:

                explicit TransactionExecutor(SqliteDatabase& db)
                    : m_db(db) {}

                template<typename Func>
                auto exec(Func&& func)
                {
                    return m_db.submitTransaction(
                        [func = std::forward<Func>(func)]() mutable {
                            return func();
                        });
                }

            };

    };

}