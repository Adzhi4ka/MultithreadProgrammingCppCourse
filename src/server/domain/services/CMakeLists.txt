add_library(server_domain_services
    STATIC
        file-acl-service.cpp
        file-lock-service.cpp
        file-content-service.cpp
        file-version-service.cpp
        file-service.cpp
        group-service.cpp
        user-service.cpp
)

target_include_directories(server_domain_services
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_domain_services
    PUBLIC
        server_domain_models
        server_infrastructure
)