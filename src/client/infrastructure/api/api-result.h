#pragma once

#include <expected>
#include <type_traits>
#include <utility>

#include <QString>

namespace client::infrastructure::api {

    struct ApiError {
        int httpStatus = 0;
        QString code;
        QString message;

        bool isNetworkError() const noexcept {
            return httpStatus == 0;
        }
    };

    template <typename T>
    using ApiResult = std::expected<T, ApiError>;

    template <typename T>
    ApiResult<std::decay_t<T>> apiSuccess(T&& value) {
        return ApiResult<std::decay_t<T>>{std::forward<T>(value)};
    }

    inline ApiResult<void> apiSuccess() {
        return ApiResult<void>{};
    }

    inline std::unexpected<ApiError> apiFailure(ApiError error) {
        return std::unexpected<ApiError>{std::move(error)};
    }

}
