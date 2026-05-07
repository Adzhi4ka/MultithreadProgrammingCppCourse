#pragma once

#include <QObject>
#include <QString>
#include <functional>
#include <vector>

#include "api-client.h"
#include "api-result.h"
#include "domain/models/remote-file.h"

namespace client::infrastructure::api {

class FileApi : public QObject {

        using RemoteFile = domain::models::RemoteFile;

        Q_OBJECT
        ApiClient& m_apiClient;

    public:

        explicit FileApi(ApiClient& apiClient, QObject* parent = nullptr);

        void getAll(std::function<void(ApiResult<std::vector<RemoteFile>>)> callback);
        void getById(qint64 fileId, std::function<void(ApiResult<RemoteFile>)> callback);
        void getByLogicalName(const QString& logicalName, std::function<void(ApiResult<RemoteFile>)> callback);
        void create(const QString& logicalName, quint32 maxVersionCount,
                    std::function<void(ApiResult<RemoteFile>)> callback);
        void rename(qint64 fileId, const QString& newLogicalName, std::function<void(ApiResult<RemoteFile>)> callback);
        void remove(qint64 fileId, std::function<void(ApiResult<void>)> callback);

    private:

        static ApiResult<RemoteFile> parseSingleFileResponse(const RawApiResponse& response);
};

}  // namespace client::infrastructure::api
