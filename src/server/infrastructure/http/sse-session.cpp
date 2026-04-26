#include "sse-session.h"

#include <boost/beast/core/bind_handler.hpp>

namespace infrastructure::http {

    SseSession::SseSession(beast::tcp_stream&& stream,
                           int64_t userId,
                           uint64_t sessionId,
                           std::function<void(int64_t, uint64_t)> onClosed)
        : m_stream(std::move(stream))
        , m_userId(userId)
        , m_sessionId(sessionId)
        , m_onClosed(std::move(onClosed)) {
    }

    void SseSession::start() {
        net::dispatch(m_stream.get_executor(),
                      beast::bind_front_handler(&SseSession::doStart, shared_from_this()));
    }

    void SseSession::doStart() {
        if (m_started || m_closed) {
            return;
        }

        m_started = true;

        enqueue("HTTP/1.1 200 OK\r\n"
                "Content-Type: text/event-stream\r\n"
                "Cache-Control: no-cache\r\n"
                "Connection: keep-alive\r\n"
                "X-Accel-Buffering: no\r\n"
                "\r\n" );

        enqueue(buildComment("connected"));
    }

    void SseSession::sendEvent(std::string eventName, std::string jsonData) {
        net::post(m_stream.get_executor(),
                  [self = shared_from_this(),
                   eventName = std::move(eventName),
                   jsonData = std::move(jsonData)]() mutable {

                    if (self->m_closed) {
                        return;
                    }
    
                    self->enqueue(buildEvent(std::move(eventName), jsonData));
                }
        );
    }

    void SseSession::sendComment(std::string comment) {
        net::post(m_stream.get_executor(),
                  [self = shared_from_this(),
                   comment = std::move(comment)]() mutable {

                        if (self->m_closed) {
                            return;
                        }

                        self->enqueue(buildComment(comment));
                    }
        );
    }

    void SseSession::close() {
        net::post(m_stream.get_executor(),
                  [self = shared_from_this()] { self->doClose(); });
    }

    int64_t SseSession::userId() const noexcept {
        return m_userId;
    }

    uint64_t SseSession::sessionId() const noexcept {
        return m_sessionId;
    }

    void SseSession::enqueue(std::string payload) {
        m_outbox.push_back(std::move(payload));

        if (!m_writeInProgress) {
            doWrite();
        }
    }

    void SseSession::doWrite() {
        if (m_closed || m_outbox.empty()) {
            m_writeInProgress = false;
            return;
        }

        m_writeInProgress = true;

        net::async_write(m_stream,
                         net::buffer(m_outbox.front()),
                         beast::bind_front_handler(&SseSession::onWrite, shared_from_this()));
    }

    void SseSession::onWrite(boost::system::error_code ec, std::size_t) {
        if (ec) {
            doClose();
            return;
        }

        m_outbox.pop_front();

        if (m_outbox.empty()) {
            m_writeInProgress = false;
            return;
        }

        doWrite();
    }

    void SseSession::doClose() {
        if (m_closed) {
            return;
        }

        m_closed = true;

        boost::system::error_code ec;
        m_stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        m_stream.socket().close(ec);

        if (m_onClosed) {
            m_onClosed(m_userId, m_sessionId);
        }
    }

    std::string SseSession::buildEvent(std::string_view eventName, std::string_view data) {
        std::size_t reserveSize = data.size() + 8;

        if (!eventName.empty()) {
            reserveSize += eventName.size() + 8;
        }

        for (auto ch : data) {
            if (ch == '\n') {
                reserveSize += 6;
            }
        }

        reserveSize += 8;

        std::string result;
        result.reserve(reserveSize);

        if (!eventName.empty()) {
            result.append("event: ");
            result.append(eventName.data(), eventName.size());
            result.push_back('\n');
        }

        std::size_t start = 0;

        for (;;) {
            const auto pos = data.find('\n', start);

            result.append("data: ");

            if (pos == std::string_view::npos) {
                result.append(data.data() + start, data.size() - start);
                result.append("\n\n");
                break;
            }

            result.append(data.data() + start, pos - start);
            result.push_back('\n');
            start = pos + 1;
        }

        return result;
    }

    std::string SseSession::buildComment(std::string_view comment) {
        std::size_t reserveSize = comment.size() + 4;

        for (auto ch : comment) {
            if (ch == '\n') {
                reserveSize += 2;
            }
        }

        reserveSize += 4;

        std::string result;
        result.reserve(reserveSize);

        std::size_t start = 0;

        for (;;) {
            const std::size_t pos = comment.find('\n', start);

            result.append(": ");

            if (pos == std::string_view::npos) {
                result.append(comment.data() + start, comment.size() - start);
                result.append("\n\n");
                break;
            }

            result.append(comment.data() + start, pos - start);
            result.push_back('\n');
            start = pos + 1;
        }

        return result;
    }

}