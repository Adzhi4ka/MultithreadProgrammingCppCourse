add_library(client_application STATIC
    client-runtime.cpp
    network-worker.cpp
)

target_include_directories(client_application
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/client
)

target_link_libraries(client_application
    PUBLIC
        client_domain_models
        client_domain_services
        client_infrastructure_api
        client_infrastructure_repositories
        Qt6::Core
        Qt6::Network
)

target_compile_features(client_application
    PUBLIC
        cxx_std_23
)
