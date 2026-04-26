find_package(SQLite3 REQUIRED)
include(FetchContent)

set(SQLITECPP_INTERNAL_SQLITE OFF CACHE BOOL "" FORCE)
set(SQLITECPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SQLITECPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SQLiteCpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG 3.3.3
)

FetchContent_MakeAvailable(SQLiteCpp)

add_library(server_infrastructure_database_sqlite STATIC
    database-factory.cpp
    schema.cpp
    sqlite-database.cpp
)

target_include_directories(server_infrastructure_database_sqlite
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_infrastructure_database_sqlite
    PUBLIC
        SQLiteCpp
        SQLite::SQLite3
)

target_compile_features(server_infrastructure_database_sqlite
    PUBLIC
        cxx_std_20
)