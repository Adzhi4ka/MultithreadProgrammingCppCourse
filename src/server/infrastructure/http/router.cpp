#include "router.h"

namespace infrastructure::http {

    std::size_t Router::RouteKeyHash::operator()(const RouteKey& key) const noexcept {
        const auto h1 = std::hash<int>{}(static_cast<int>(key.method));
        const auto h2 = std::hash<std::string>{}(key.path);

        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }

    void Router::add(http::verb method, std::string path, Handler handler) {
        m_routes.emplace(RouteKey{method, std::move(path)}, std::move(handler));
    }

    void Router::dispatch(RouteContext& context) const {
        const auto path = normalizePath(context.request());

        const auto it = m_routes.find(RouteKey{context.request().method(), path});
        if (it == m_routes.end()) {
            context.reply(makeTextResponse(
                http::status::not_found,
                "Route not found",
                context.request().version(),
                context.request().keep_alive()
            ));
            return;
        }

        it->second(context);
    }

    std::string Router::normalizePath(const Request& request) {
        std::string target(request.target());

        const auto pos = target.find('?');
        if (pos != std::string::npos) {
            target.resize(pos);
        }

        if (target.empty()) {
            return "/";
        }

        return target;
    }

    Response makeTextResponse(http::status status,
                              std::string body,
                              unsigned version,
                              bool keepAlive) {
        Response response{status, version};
        response.set(http::field::content_type, "text/plain; charset=utf-8");
        response.keep_alive(keepAlive);
        response.body() = std::move(body);
        response.prepare_payload();
        return response;
    }

    Response makeJsonResponse(http::status status,
                              std::string body,
                              unsigned version,
                              bool keepAlive) {
        Response response{status, version};
        response.set(http::field::content_type, "application/json; charset=utf-8");
        response.keep_alive(keepAlive);
        response.body() = std::move(body);
        response.prepare_payload();
        return response;
    }

}