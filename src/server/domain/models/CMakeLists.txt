cmake_minimum_required(VERSION 3.20)

add_library(server_domain_models INTERFACE)

target_include_directories(server_domain_models
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src/server
)