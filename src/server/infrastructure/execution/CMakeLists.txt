if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost REQUIRED COMPONENTS system)

add_library(server_infrastructure_execution INTERFACE)

target_include_directories(server_infrastructure_execution
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_infrastructure_execution
    INTERFACE
        Boost::boost
        Boost::system
)