add_subdirectory(infrastructure)
add_subdirectory(domain)
add_subdirectory(presentation)

add_executable(server_app
    main.cpp
)

target_link_libraries(server_app
    PRIVATE
        server_infrastructure
        server_domain
        server_presentation
)