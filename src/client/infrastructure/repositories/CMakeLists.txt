add_library(client_infrastructure_repositories STATIC
    file-repository.cpp
    group-repository.cpp
    session-repository.cpp
)

target_include_directories(client_infrastructure_repositories
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/client
)

target_link_libraries(client_infrastructure_repositories
    PUBLIC
        client_domain_models
        Qt6::Core
)

target_compile_features(client_infrastructure_repositories
    PUBLIC
        cxx_std_23
)
