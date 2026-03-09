cmake_minimum_required(VERSION 3.20)
project(infrastructure CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    target_compile_options(sqlite_orm INTERFACE -Wno-deprecated-declarations -Wno-unused-local-typedefs)
endif()

add_library(infrastructure INTERFACE)

target_include_directories(infrastructure
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(infrastructure
    INTERFACE
        SQLite::SQLite3
        sqlite_orm
        Boost::boost
)

add_executable(test_compile ${CMAKE_CURRENT_SOURCE_DIR}/test-compile.cpp)
target_link_libraries(test_compile PRIVATE infrastructure)