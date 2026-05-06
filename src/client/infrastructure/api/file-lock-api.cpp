#include "file-lock-api.h"

#include "json-utils.h"

#include <QJsonObject>
#include <QUrlQuery>

namespace {
    using FileLock = client::domain::models::FileLock;
}

namespace client::infrastructure::api {

    FileLockApi::FileLockApi(ApiClient& apiClient, QObject* parent)
        : QObject(parent),
          m_apiClient(apiClient) {}

    void FileLockApi::acquire(qint64 fileId, qint64 lockDurationSec, std::function<void(ApiResult<FileLock>)> callback) {
        QJsonObject body{{"fileId", fileId}};
        if (lockDurationSec > 0) {
            body.insert("lockDurationSec", lockDurationSec);
        }

        m_apiClient.postJson("/api/file-locks", body, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseLockResponse(response));
        });
    }

    void FileLockApi::renew(qint64 fileId, qint64 lockToken, qint64 lockDurationSec, std::function<void(ApiResult<void>)> callback) {
        QJsonObject body{
            {"fileId", fileId},
            {"lockToken", lockToken},
        };

        if (lockDurationSec > 0) {
            body.insert("lockDurationSec", lockDurationSec);
        }

        m_apiClient.putJson("/api/file-locks/renew", body, [callback = std::move(callback)](RawApiResponse response) mutable {
            if (!response.isSuccessStatus()) {
                callback(apiFailure(makeHttpError(response, "failed to renew file lock")));
                return;
            }

            callback(apiSuccess());
        });
    }

    void FileLockApi::release(qint64 fileId, qint64 lockToken, std::function<void(ApiResult<void>)> callback) {
        QUrlQuery query;
        query.addQueryItem("fileId", QString::number(fileId));
        query.addQueryItem("lockToken", QString::number(lockToken));

        m_apiClient.deleteRequest("/api/file-locks", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            if (!response.isSuccessStatus()) {
                callback(apiFailure(makeHttpError(response, "failed to release file lock")));
                return;
            }

            callback(apiSuccess());
        });
    }

    void FileLockApi::getActive(qint64 fileId, std::function<void(ApiResult<FileLock>)> callback) {
        QUrlQuery query;
        query.addQueryItem("fileId", QString::number(fileId));

        m_apiClient.get("/api/file-locks/active", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseLockResponse(response));
        });
    }

    ApiResult<FileLock> FileLockApi::parseLockResponse(const RawApiResponse& response) {
        if (!response.isSuccessStatus()) {
            return apiFailure(makeHttpError(response, "file lock operation failed"));
        }

        QString parseError;
        const auto object = parseJsonObject(response.body, &parseError);
        if (!object) {
            return apiFailure(ApiError{response.httpStatus, "invalid_json", parseError});
        }

        return apiSuccess(parseFileLock(*object));
    }

}
