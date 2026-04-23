#pragma once

#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

#include <cstddef>
#include <future>
#include <type_traits>
#include <utility>

namespace infrastructure::execution {

    class ThreadPool {

            boost::asio::thread_pool m_pool;

        public:

            explicit ThreadPool(std::size_t threadCount)
                : m_pool(threadCount == 0 ? 1 : threadCount) {
            }

            ~ThreadPool() {
                m_pool.join();
            }

            ThreadPool(const ThreadPool&) = delete;
            ThreadPool& operator=(const ThreadPool&) = delete;

            template <class F>
            void post(F&& fn) {
                boost::asio::post(m_pool, std::forward<F>(fn));
            }

            template <class F>
            auto submit(F&& fn) -> std::future<std::invoke_result_t<std::decay_t<F>&>> {
                using Fn = std::decay_t<F>;
                using Result = std::invoke_result_t<Fn&>;

                std::packaged_task<Result()> task(std::forward<F>(fn));
                auto future = task.get_future();

                boost::asio::post(m_pool,
                                  [task = std::move(task)]() mutable {
                                      task();
                                  });

                return future;
            }

    };

}