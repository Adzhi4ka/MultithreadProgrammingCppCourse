add_subdirectory(database)
add_subdirectory(file-storage)

add_library(server_infrastructure INTERFACE)

target_link_libraries(server_infrastructure
    INTERFACE
        server_infrastructure_database
        server_infrastructure_file_storage
)