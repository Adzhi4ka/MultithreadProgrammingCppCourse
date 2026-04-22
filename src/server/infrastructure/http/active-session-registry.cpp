#include "active-session-registry.h"
#include "sse-session.h"
#include <shared_mutex>

namespace infrastructure::http {

    uint64_t ActiveSessionRegistry::nextSessionId() noexcept {
        return m_nextSessionId.fetch_add(1, std::memory_order_relaxed);
    }

    void ActiveSessionRegistry::add(const std::shared_ptr<SseSession>& session) {
        std::lock_guard lock(m_mutex);
        m_sessions[session->userId()][session->sessionId()] = session;
    }

    void ActiveSessionRegistry::remove(int64_t userId, uint64_t sessionId) {
        std::lock_guard lock(m_mutex);

        const auto userIt = m_sessions.find(userId);
        if (userIt == m_sessions.end()) {
            return;
        }

        userIt->second.erase(sessionId);

        if (userIt->second.empty()) {
            m_sessions.erase(userIt);
        }
    }

    void ActiveSessionRegistry::publishToUser(int64_t userId,
                                              const std::string& eventName,
                                              const std::string& data) {
        auto sessions = collectUserSessions(userId);

        for (auto& session : sessions) {
            session->sendEvent(eventName, data);
        }
    }

    void ActiveSessionRegistry::publishToAll(const std::string& eventName,
                                             const std::string& data) {
        auto sessions = collectAllSessions();

        for (auto& session : sessions) {
            session->sendEvent(eventName, data);
        }
    }

    std::vector<std::shared_ptr<SseSession>> ActiveSessionRegistry::collectUserSessions(int64_t userId) {
        std::vector<std::shared_ptr<SseSession>> result;

        std::shared_lock sharedLock(m_mutex);

        const auto userIt = m_sessions.find(userId);
        if (userIt == m_sessions.end()) {
            return result;
        }

        result.reserve(userIt->second.size());

        for (auto& [sessionId, session] : userIt->second) {
            result.emplace_back(session);
        }

        return result;
    }

    std::vector<std::shared_ptr<SseSession>> ActiveSessionRegistry::collectAllSessions() {
        std::vector<std::shared_ptr<SseSession>> result;

        std::shared_lock sharedLock(m_mutex);

        for (auto& [userId, userSessions] : m_sessions) {
            result.reserve(result.size() + userSessions.size());

            for (auto& [sessionId, session] : userSessions) {
                result.push_back(session);
            }
        }

        return result;
    }

}