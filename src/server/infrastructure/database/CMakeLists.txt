cmake_minimum_required(VERSION 3.20)

find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED)

include(FetchContent)

FetchContent_Declare(
    sqlite_orm
    GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
    GIT_TAG master
)

FetchContent_MakeAvailable(sqlite_orm)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(sqlite_orm INTERFACE
        -Wno-deprecated-declarations
        -Wno-unused-local-typedefs
    )
endif()

add_subdirectory(repositories)
add_subdirectory(sqlite)

add_library(server_infrastructure_database INTERFACE)

target_link_libraries(server_infrastructure_database
    INTERFACE
        server_infrastructure_database_repositories
        server_infrastructure_database_sqlite
)