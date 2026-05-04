#pragma once

#include "api-client.h"
#include "api-result.h"

#include "domain/models/remote-file.h"

#include <QObject>
#include <QString>

#include <functional>
#include <vector>

namespace client::infrastructure::api {

    class FileApi final : public QObject {
        Q_OBJECT

    public:
        using FileResult = ApiResult<domain::models::RemoteFile>;
        using FilesResult = ApiResult<std::vector<domain::models::RemoteFile>>;
        using FileCallback = std::function<void(FileResult)>;
        using FilesCallback = std::function<void(FilesResult)>;
        using VoidCallback = std::function<void(ApiResult<void>)>;

        explicit FileApi(ApiClient& apiClient, QObject* parent = nullptr);

        void getAll(FilesCallback callback);
        void getById(qint64 fileId, FileCallback callback);
        void getByLogicalName(const QString& logicalName, FileCallback callback);
        void create(const QString& logicalName, quint32 maxVersionCount, FileCallback callback);
        void rename(qint64 fileId, const QString& newLogicalName, FileCallback callback);
        void remove(qint64 fileId, VoidCallback callback);

    private:
        static FileResult parseSingleFileResponse(const RawApiResponse& response);

    private:
        ApiClient& m_apiClient;
    };

}
