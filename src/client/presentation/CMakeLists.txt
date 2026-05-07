add_library(client_presentation STATIC
    login-dialog.cpp
    main-window.cpp
    editor-window.cpp
    ui-format.cpp
    styles/app-style.cpp
    widgets/file-tree-widget.cpp
    widgets/file-info-widget.cpp
    dialogs/group-management-dialog.cpp
    dialogs/create-file-dialog.cpp
)

qt_add_resources(client_presentation client_presentation_resources
    PREFIX
        "/client"
    FILES
        styles/dark.qss
)

target_include_directories(client_presentation
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/client
)

target_link_libraries(client_presentation
    PUBLIC
        client_domain_models
        client_application
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
