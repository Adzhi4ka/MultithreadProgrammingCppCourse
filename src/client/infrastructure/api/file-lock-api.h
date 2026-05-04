#pragma once

#include "api-client.h"
#include "api-result.h"

#include "domain/models/file-lock.h"

#include <QObject>

#include <functional>

namespace client::infrastructure::api {

    class FileLockApi final : public QObject {
        Q_OBJECT

    public:
        using LockResult = ApiResult<domain::models::FileLock>;
        using LockCallback = std::function<void(LockResult)>;
        using VoidCallback = std::function<void(ApiResult<void>)>;

        explicit FileLockApi(ApiClient& apiClient, QObject* parent = nullptr);

        void acquire(qint64 fileId, qint64 lockDurationSec, LockCallback callback);
        void renew(qint64 fileId, qint64 lockToken, qint64 lockDurationSec, VoidCallback callback);
        void release(qint64 fileId, qint64 lockToken, VoidCallback callback);
        void getActive(qint64 fileId, LockCallback callback);

    private:
        static LockResult parseLockResponse(const RawApiResponse& response);

    private:
        ApiClient& m_apiClient;
    };

}
