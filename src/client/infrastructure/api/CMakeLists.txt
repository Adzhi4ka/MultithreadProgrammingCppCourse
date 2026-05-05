add_library(client_infrastructure_api STATIC
    api-client.cpp
    json-utils.cpp
    auth-api.cpp
    file-api.cpp
    file-content-api.cpp
    file-lock-api.cpp
    file-acl-api.cpp
    group-api.cpp
    file-version-api.cpp

    api-result.h
    api-client.h
    json-utils.h
    auth-api.h
    file-api.h
    file-content-api.h
    file-lock-api.h
    file-acl-api.h
    group-api.h
    file-version-api.h
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