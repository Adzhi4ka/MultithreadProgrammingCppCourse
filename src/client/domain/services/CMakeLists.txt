add_library(client_domain_services STATIC
    auth-service.cpp
    file-catalog-service.cpp
    file-workspace-service.cpp
    group-sharing-service.cpp
    notification-service.cpp
)

target_include_directories(client_domain_services
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/client
)

target_link_libraries(client_domain_services
    PUBLIC
        client_domain_models
        client_infrastructure_api
        client_infrastructure_repositories
        Qt6::Core
        Qt6::Network
)

target_compile_features(client_domain_services
    PUBLIC
        cxx_std_23
)
