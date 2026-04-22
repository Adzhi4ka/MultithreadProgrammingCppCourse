add_library(server_presentation_http STATIC
    test-controller.cpp
)

target_include_directories(server_presentation_http
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_presentation_http
    PUBLIC
        server_infrastructure
)