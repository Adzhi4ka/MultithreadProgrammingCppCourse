# Qt MVP client

MVP-клиент собирается отдельной целью:

```bash
cmake --preset debug
cmake --build --preset debug --target file_storage_qt_client
./build/debug/src/client/file_storage_qt_client
```

Что есть:

- login/register через `/api/auth/*`;
- псевдофайловое дерево из `fullLogicalName`;
- метаинформация файла: id, версия, createdBy, createdAt, ACL, lock;
- открыть текущую версию readonly;
- взять lock и открыть файл на редактирование;
- save через `PUT /api/files/content`;
- release/renew lock;
- create/rename/delete file;
- просмотр старых версий через `/api/file-versions` и readonly-открытие версии.

Что сознательно не делалось для MVP:

- отдельный `UserController` на сервере;
- нормальная таблица пользователей;
- исправление серверной бизнес-логики lock/ACL;
- persistent local storage — состояние хранится in-memory.
