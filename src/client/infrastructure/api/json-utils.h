#pragma once

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <optional>
#include <vector>

#include "api-result.h"
#include "domain/models/file-acl.h"
#include "domain/models/file-lock.h"
#include "domain/models/file-version.h"
#include "domain/models/group.h"
#include "domain/models/remote-file.h"
#include "domain/models/user-profile.h"
#include "domain/models/user-session.h"

namespace client::infrastructure::api {

struct RawApiResponse;

std::optional<QJsonObject> parseJsonObject(const QByteArray& bytes, QString* errorMessage = nullptr);
std::optional<QJsonArray> getArrayField(const QJsonObject& object, QStringView fieldName);
std::optional<qint64> getInt64Field(const QJsonObject& object, QStringView fieldName);
std::optional<quint32> getUInt32Field(const QJsonObject& object, QStringView fieldName);
std::optional<QString> getStringField(const QJsonObject& object, QStringView fieldName);

ApiError makeHttpError(const RawApiResponse& response, QString fallbackMessage);

domain::models::UserSession parseUserSession(const QJsonObject& object);
domain::models::UserProfile parseUserProfile(const QJsonObject& object);
domain::models::RemoteFile parseRemoteFile(const QJsonObject& object);
domain::models::FileVersion parseFileVersion(const QJsonObject& object);
domain::models::FileLock parseFileLock(const QJsonObject& object);
domain::models::FileAcl parseFileAcl(const QJsonObject& object);
domain::models::UserFileAcl parseUserFileAcl(const QJsonObject& object);
domain::models::Group parseGroup(const QJsonObject& object);

std::vector<domain::models::RemoteFile> parseRemoteFileItems(const QJsonObject& root);
std::vector<domain::models::FileVersion> parseFileVersionItems(const QJsonObject& root);
std::vector<domain::models::FileAcl> parseFileAclItems(const QJsonObject& root);
std::vector<domain::models::Group> parseGroupItems(const QJsonObject& root);

}  // namespace client::infrastructure::api
