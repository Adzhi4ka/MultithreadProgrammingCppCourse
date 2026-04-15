add_library(server_infrastructure_database_repositories STATIC
    file-acl-repository.cpp
    file-lock-repository.cpp
    file-repository.cpp
    file-version-repository.cpp
    group-repository.cpp
    user-group-repository.cpp
    user-repository.cpp
)

target_include_directories(server_infrastructure_database_repositories
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_infrastructure_database_repositories
    PUBLIC
        server_domain_models
        server_infrastructure_database_sqlite
)

target_compile_features(server_infrastructure_database_repositories 
    PUBLIC 
        cxx_std_20)