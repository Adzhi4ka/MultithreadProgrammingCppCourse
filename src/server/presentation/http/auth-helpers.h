#pragma once

#include "infrastructure/http/router.h"
#include "infrastructure/security/auth-token-store.h"

#include <optional>
#include <string>
#include <string_view>

namespace presentation::http {

    inline std::optional<std::string_view> extractBearerToken(const infrastructure::http::Request& request) {
        namespace beast = boost::beast;
        namespace http = beast::http;

        const auto value = request[http::field::authorization];
        if (value.empty()) {
            return std::nullopt;
        }

        std::string_view auth{value.data(), value.size()};
        static constexpr std::string_view kPrefix = "Bearer ";

        if (!auth.starts_with(kPrefix)) {
            return std::nullopt;
        }

        auth.remove_prefix(kPrefix.size());

        if (auth.empty()) {
            return std::nullopt;
        }

        return auth;
    }

    inline std::optional<int64_t> authenticateUserId(const infrastructure::http::Request& request,
                                                     const infrastructure::security::AuthTokenStore& tokenStore) {
        const auto token = extractBearerToken(request);
        if (!token) {
            return std::nullopt;
        }

        return tokenStore.resolveUserId(*token);
    }

    inline infrastructure::http::Response makeUnauthorizedResponse(const infrastructure::http::Request& request) {
        namespace beast = boost::beast;
        namespace http = beast::http;

        auto response = infrastructure::http::makeJsonResponse(
            http::status::unauthorized,
            R"({"error":"unauthorized"})",
            request.version(),
            request.keep_alive()
        );

        response.set(http::field::www_authenticate, "Bearer");
        return response;
    }

}