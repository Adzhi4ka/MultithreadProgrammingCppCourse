add_library(domain_models INTERFACE)

target_include_directories(domain_models
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)