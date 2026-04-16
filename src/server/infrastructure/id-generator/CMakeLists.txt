add_library(server_infrastructure_id_generator INTERFACE)

target_include_directories(server_infrastructure_repositories
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src/server
)