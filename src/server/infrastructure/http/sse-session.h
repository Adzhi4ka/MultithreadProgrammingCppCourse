#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <string>

namespace infrastructure::http {

namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;

class SseSession : public std::enable_shared_from_this<SseSession> {

        beast::tcp_stream m_stream;

        int64_t m_userId;
        uint64_t m_sessionId;

        std::deque<std::string> m_outbox;

        bool m_started{false};
        bool m_writeInProgress{false};
        bool m_closed{false};

        std::function<void(int64_t, uint64_t)> m_onClosed;
        net::strand<net::any_io_executor> m_strand;

    public:

        SseSession(beast::tcp_stream&& stream, int64_t userId, uint64_t sessionId,
                   std::function<void(int64_t, uint64_t)> onClosed);

        void start();

        void sendEvent(std::string eventName, std::string jsonData);
        void sendComment(std::string comment);
        void close();

        int64_t userId() const noexcept;
        uint64_t sessionId() const noexcept;

    private:

        void doStart();

        void enqueue(std::string payload);

        void doWrite();
        void onWrite(boost::system::error_code ec, std::size_t bytesTransferred);

        void doClose();

        static std::string buildEvent(std::string_view eventName, std::string_view data);
        static std::string buildComment(std::string_view comment);
};

}  // namespace infrastructure::http