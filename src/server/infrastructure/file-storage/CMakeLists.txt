add_library(server_infrastructure_file_storage STATIC
    file-storage.cpp
)

target_include_directories(server_infrastructure_file_storage
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/server
)