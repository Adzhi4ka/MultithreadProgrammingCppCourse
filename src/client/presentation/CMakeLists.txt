add_library(client_presentation STATIC
    login-dialog.cpp
    main-window.cpp

    login-dialog.h
    main-window.h
)

target_include_directories(client_presentation
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/client
)

target_link_libraries(client_presentation
    PUBLIC
        client_domain_models
        client_infrastructure_api
        client_infrastructure_repositories
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
)

target_compile_features(client_presentation
    PUBLIC
        cxx_std_23
)
