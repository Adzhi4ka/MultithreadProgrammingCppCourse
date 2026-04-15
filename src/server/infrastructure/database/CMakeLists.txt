cmake_minimum_required(VERSION 3.20)

find_package(Boost REQUIRED)

add_subdirectory(repositories)
add_subdirectory(sqlite)

add_library(server_infrastructure_database INTERFACE)

target_link_libraries(server_infrastructure_database
    INTERFACE
        server_infrastructure_database_repositories
        server_infrastructure_database_sqlite
)