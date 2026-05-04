#pragma once

#include <boost/signals2.hpp>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <variant>

namespace domain::notifications::events {

    /*
    
        Заготовка под серверную часть, может быть на это прикручу еще кеширование
    
    */

    template<typename ...Events>
    class EventBus {

            template<typename Event>
            using Signal = boost::signals2::signal<void(const Event&)>;
            std::tuple<Signal<Events>...> m_signals;

            using EventVariant = std::variant<Events...>;
            std::queue<EventVariant> m_eventsQueue;

            std::mutex m_queueMutex;
            std::condition_variable m_queueCondVar;

        public:

            template<typename Event>
            void post(Event&& event) {
                {
                    std::lock_guard lock(m_queueMutex);
                    m_eventsQueue.emplace(std::move(event));
                }

                m_queueCondVar.notify_one();
            }

            template<typename Event, typename Handler>
            auto subscribe(Handler&& handler) {
                auto& concreteSignal = std::get<Signal<Event>>(m_signals); 
                return concreteSignal.connect(std::forward<Handler>(handler));
            }

        private:

            std::jthread m_workerJthread;

            void workerTask(std::stop_token stopToken) {
                while (!stopToken.stop_requested()) {
                    EventVariant event;
                    
                    {
                        std::unique_lock ul{m_queueMutex};
                        m_queueCondVar.wait(ul, [this, &stopToken] {
                            return stopToken.stop_requested() || !m_eventsQueue.empty();
                        });

                        if (stopToken.stop_requested() && m_eventsQueue.empty()) {
                            return;
                        }

                        event = std::move(m_eventsQueue.front());
                        m_eventsQueue.pop();
                    }

                    std::visit([this](const auto& event) {
                        using Event = std::decay_t<decltype(event)>;

                        auto& concreteSignal = std::get<Signal<Event>>(m_signals); 
                        concreteSignal(event);
                    }, event);
                }
            }

        public:

            void start() {
                m_workerJthread = std::jthread([this](std::stop_token st) {
                    workerTask(st);
                });
            }

            void stop() {
                if (m_workerJthread.joinable()) {
                    m_workerJthread.request_stop();
                    m_queueCondVar.notify_all();
                }
            }

           ~EventBus() {
                stop();
            }
    };

}