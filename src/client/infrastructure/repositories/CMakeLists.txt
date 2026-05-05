add_library(client_infrastructure_repositories INTERFACE)

target_include_directories(client_infrastructure_repositories
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src/client
)

target_link_libraries(client_infrastructure_repositories
    INTERFACE
        client_domain_models
)
