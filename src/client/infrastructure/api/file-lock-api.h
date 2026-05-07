#pragma once

#include <QObject>
#include <functional>

#include "api-client.h"
#include "api-result.h"
#include "domain/models/file-lock.h"

namespace client::infrastructure::api {

class FileLockApi : public QObject {

        using FileLock = domain::models::FileLock;

        Q_OBJECT
        ApiClient& m_apiClient;

    public:

        explicit FileLockApi(ApiClient& apiClient, QObject* parent = nullptr);

        void acquire(qint64 fileId, qint64 lockDurationSec, std::function<void(ApiResult<FileLock>)> callback);
        void renew(qint64 fileId, qint64 lockToken, qint64 lockDurationSec,
                   std::function<void(ApiResult<void>)> callback);
        void release(qint64 fileId, qint64 lockToken, std::function<void(ApiResult<void>)> callback);
        void getActive(qint64 fileId, std::function<void(ApiResult<FileLock>)> callback);

    private:

        static ApiResult<FileLock> parseLockResponse(const RawApiResponse& response);
};

}  // namespace client::infrastructure::api
