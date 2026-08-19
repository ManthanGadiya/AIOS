add_library(aios_core STATIC
    aios/core/types.cpp
)

target_include_directories(aios_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)