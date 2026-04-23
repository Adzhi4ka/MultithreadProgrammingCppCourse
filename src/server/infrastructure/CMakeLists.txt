add_subdirectory(database)
add_subdirectory(file-storage)
add_subdirectory(repositories)
add_subdirectory(id-generator)
add_subdirectory(security)
add_subdirectory(event)
add_subdirectory(execution)
add_subdirectory(http)

add_library(server_infrastructure INTERFACE)

target_link_libraries(server_infrastructure
    INTERFACE
        server_infrastructure_database
        server_infrastructure_file_storage
        server_infrastructure_repositories
        server_infrastructure_security
        server_infrastructure_event
        server_infrastructure_execution
        server_infrastructure_http
)