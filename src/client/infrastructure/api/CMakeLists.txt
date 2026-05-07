add_library(client_infrastructure_api STATIC
    api-client.cpp
    json-utils.cpp
    auth-api.cpp
    user-api.cpp
    file-api.cpp
    file-content-api.cpp
    file-lock-api.cpp
    file-acl-api.cpp
    group-api.cpp
    file-version-api.cpp
    notification-stream-client.cpp
    remote-api-gateway.cpp
)

target_include_directories(client_infrastructure_api
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/client
)

target_link_libraries(client_infrastructure_api
    PUBLIC
        client_domain_models
        Qt6::Core
        Qt6::Network
)

target_compile_features(client_infrastructure_api
    PUBLIC
        cxx_std_23
)
