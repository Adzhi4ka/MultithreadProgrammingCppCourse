add_subdirectory(http)

add_library(server_presentation INTERFACE)

target_link_libraries(server_presentation
    INTERFACE
        server_presentation_http
)