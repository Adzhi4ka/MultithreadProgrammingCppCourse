#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace infrastructure::http {

    class SseSession;

    class ActiveSessionRegistry {

            using SessionMap = std::unordered_map<uint64_t, std::shared_ptr<SseSession>>;

            std::unordered_map<int64_t, SessionMap> m_sessions;
            std::mutex m_mutex;

            std::atomic_uint64_t m_nextSessionId {1};

        public:

            uint64_t nextSessionId() noexcept;

            void add(const std::shared_ptr<SseSession>& session);
            void remove(int64_t userId, uint64_t sessionId);

            void publishToUser(int64_t userId,
                               const std::string& eventName,
                               const std::string& data);

            void publishToAll(const std::string& eventName,
                              const std::string& data);

        private:

            std::vector<std::shared_ptr<SseSession>> collectUserSessions(int64_t userId);
            std::vector<std::shared_ptr<SseSession>> collectAllSessions();

    };

}