#pragma once

#include "infrastructure/execution/thread-pool.h"
#include "infrastructure/http/router.h"

#include <boost/beast/http.hpp>

#include <utility>

namespace presentation::http {

    template <class Work>
    void dispatchToWorker(infrastructure::http::RouteContext& ctx,
                          infrastructure::execution::ThreadPool& threadPool,
                          Work&& work) {
        namespace http = boost::beast::http;

        const auto version = ctx.request().version();
        const bool keepAlive = ctx.request().keep_alive();

        auto responder = ctx.deferResponse();

        threadPool.post([responder = std::move(responder),
                         work = std::forward<Work>(work),
                         version,
                         keepAlive]() mutable {
            try {
                responder.send(work());
            } catch (...) {
                responder.send(infrastructure::http::makeJsonResponse(http::status::internal_server_error,
                                                                     R"({"error":"internal_error"})",
                                                                     version,
                                                                     keepAlive));
            }
        });
    }

}