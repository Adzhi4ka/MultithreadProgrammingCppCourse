# Qt client MVP

Клиент сделан под текущий серверный API и повторяет серверное разбиение по слоям.

## Потоки

- UI thread: `presentation/*`, окна и виджеты.
- Network thread: `application::NetworkWorker`, `infrastructure::api::RemoteApiGateway`, `QNetworkAccessManager`, REST API и SSE stream.
- Internal thread: `domain::services::*`, in-memory repositories и сборка клиентского состояния.

Единого внутреннего worker-класса нет: `ClientRuntime` постит команды напрямую в нужный доменный сервис через общий internal `QObject`-контекст.

## Слои

```text
src/client/domain
    models/
    services/

src/client/infrastructure
    api/
    repositories/

src/client/application
    client-runtime.*
    network-worker.*

src/client/presentation
    main-window.*
    editor-window.*
    widgets/
    dialogs/
```

## Notifications stream

Клиент подключается к:

```http
GET /api/notifications/stream
```

SSE-события разбираются в `NotificationStreamClient`. Основное окно обновляет список файлов при:

- `file_created`;
- `file_locked`;
- `group_assigned`.
