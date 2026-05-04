add_library(client_domain_models INTERFACE)

target_include_directories(client_domain_models
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src/client
)