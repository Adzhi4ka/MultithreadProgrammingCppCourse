add_library(server_domain_notification INTERFACE)

target_include_directories(server_infrastructure_database_repositories
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_infrastructure_database_repositories
    INTERFACE
        server_domain_models
)