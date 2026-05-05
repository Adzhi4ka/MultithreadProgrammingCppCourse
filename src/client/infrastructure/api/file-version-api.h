#pragma once

#include "api-client.h"
#include "api-result.h"

#include "domain/models/file-version.h"

#include <QObject>
#include <QString>

#include <functional>
#include <vector>

namespace client::infrastructure::api {

    class FileVersionApi final : public QObject {
        Q_OBJECT

    public:
        using VersionResult = ApiResult<domain::models::FileVersion>;
        using VersionsResult = ApiResult<std::vector<domain::models::FileVersion>>;
        using VersionCallback = std::function<void(VersionResult)>;
        using VersionsCallback = std::function<void(VersionsResult)>;

        explicit FileVersionApi(ApiClient& apiClient, QObject* parent = nullptr);

        void create(qint64 fileId, const QString& logicalNameSnapshot, VersionCallback callback);
        void getCurrent(qint64 fileId, VersionCallback callback);
        void getAll(qint64 fileId, VersionsCallback callback);

    private:
        static VersionResult parseVersionResponse(const RawApiResponse& response);
        static VersionsResult parseVersionsResponse(const RawApiResponse& response);

    private:
        ApiClient& m_apiClient;
    };

}
