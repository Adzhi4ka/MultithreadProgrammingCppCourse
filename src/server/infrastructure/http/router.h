#pragma once

#include <boost/beast/http.hpp>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "route-context.h"

namespace infrastructure::http {

namespace beast = boost::beast;
namespace http = beast::http;

class Router {

    public:

        using Handler = std::function<void(RouteContext&)>;

    private:

        struct RouteKey {
                http::verb method;
                std::string_view path;

                bool operator==(const RouteKey& other) const noexcept = default;
        };

        struct RouteKeyHash {
                std::size_t operator()(const RouteKey& key) const noexcept;
        };

        std::unordered_map<RouteKey, Handler, RouteKeyHash> m_routes;

    public:

        void add(http::verb method, std::string_view path, Handler handler);

        void dispatch(RouteContext& context) const;

    private:

        static std::string_view normalizePath(const Request& request);
};

Response makeTextResponse(http::status status, std::string body, unsigned version, bool keepAlive);

Response makeJsonResponse(http::status status, std::string body, unsigned version, bool keepAlive);

}  // namespace infrastructure::http