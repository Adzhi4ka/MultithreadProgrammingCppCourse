#pragma once

#include <boost/beast/http.hpp>
#include <string>
#include <string_view>

#include "domain/services/service-result.h"
#include "infrastructure/http/router.h"

namespace presentation::http {

inline infrastructure::http::Response makeBadRequestResponse(unsigned version, bool keepAlive, std::string_view error) {
    namespace beast = boost::beast;
    namespace http = beast::http;

    return infrastructure::http::makeJsonResponse(http::status::bad_request,
                                                  "{\"error\":\"" + std::string(error) + "\"}", version, keepAlive);
}

inline infrastructure::http::Response makeBadRequestResponse(const infrastructure::http::Request& request,
                                                             std::string_view error) {
    return makeBadRequestResponse(request.version(), request.keep_alive(), error);
}

inline infrastructure::http::Response makeServiceErrorResponse(unsigned version, bool keepAlive,
                                                               domain::services::ServiceError error,
                                                               std::string_view forbiddenMessage = "forbidden") {
    namespace beast = boost::beast;
    namespace http = beast::http;

    switch (error) {
        case domain::services::ServiceError::Conflict:
            return infrastructure::http::makeJsonResponse(http::status::conflict, R"({"error":"conflict"})", version,
                                                          keepAlive);

        case domain::services::ServiceError::Forbidden:
            return infrastructure::http::makeJsonResponse(
                http::status::forbidden, "{\"error\":\"" + std::string(forbiddenMessage) + "\"}", version, keepAlive);

        case domain::services::ServiceError::NotFound:
            return infrastructure::http::makeJsonResponse(http::status::not_found, R"({"error":"not_found"})", version,
                                                          keepAlive);

        case domain::services::ServiceError::InternalError:
        default:
            return infrastructure::http::makeJsonResponse(http::status::internal_server_error,
                                                          R"({"error":"internal_error"})", version, keepAlive);
    }
}

inline infrastructure::http::Response makeServiceErrorResponse(const infrastructure::http::Request& request,
                                                               domain::services::ServiceError error,
                                                               std::string_view forbiddenMessage = "forbidden") {
    return makeServiceErrorResponse(request.version(), request.keep_alive(), error, forbiddenMessage);
}

}  // namespace presentation::http