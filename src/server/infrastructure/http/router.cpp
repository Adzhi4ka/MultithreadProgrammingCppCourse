#include "router.h"

#include <string_view>
#include <utility>

namespace infrastructure::http {

std::size_t Router::RouteKeyHash::operator()(const RouteKey& key) const noexcept {
    const auto h1 = std::hash<int>{}((int)key.method);
    const auto h2 = std::hash<std::string_view>{}(key.path);

    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
}

void Router::add(http::verb method, std::string_view path, Handler handler) {
    m_routes.emplace(RouteKey{method, path}, std::move(handler));
}

void Router::dispatch(RouteContext& context) const {
    const auto path = normalizePath(context.request());

    const auto it = m_routes.find(RouteKey{context.request().method(), path});
    if (it == m_routes.end()) {
        context.reply(makeTextResponse(http::status::not_found, "Route not found", context.request().version(),
                                       context.request().keep_alive()));
        return;
    }

    it->second(context);
}

std::string_view Router::normalizePath(const Request& request) {
    auto target(request.target());

    const auto pos = target.find('?');
    if (pos != std::string::npos) {
        target = target.substr(0, pos);
    }

    if (target.empty()) {
        return "/";
    }

    return target;
}

Response makeTextResponse(http::status status, std::string body, unsigned version, bool keepAlive) {

    StringResponse response{status, version};
    response.set(http::field::content_type, "text/plain; charset=utf-8");
    response.keep_alive(keepAlive);
    response.body() = std::move(body);
    response.prepare_payload();
    return Response{std::move(response)};
}

Response makeJsonResponse(http::status status, std::string body, unsigned version, bool keepAlive) {

    StringResponse response{status, version};
    response.set(http::field::content_type, "application/json; charset=utf-8");
    response.keep_alive(keepAlive);
    response.body() = std::move(body);
    response.prepare_payload();
    return Response{std::move(response)};
}

}  // namespace infrastructure::http