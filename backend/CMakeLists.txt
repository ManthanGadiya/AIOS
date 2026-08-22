cmake_minimum_required(VERSION 3.16)

project(aios_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_subdirectory(src)
add_subdirectory(tests)

# Server executable
add_executable(aios_server
    src/aios/server/main.cpp
)
target_link_libraries(aios_server PRIVATE aios_core)
if(WIN32)
    target_link_libraries(aios_server PRIVATE ws2_32)
endif()