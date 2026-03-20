add_library(server_infrastructure_database_repositories INTERFACE)

target_include_directories(server_infrastructure_database_repositories
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_infrastructure_database_repositories
    INTERFACE
        server_domain_models
)

target_compile_features(server_infrastructure_database_repositories INTERFACE cxx_std_20)