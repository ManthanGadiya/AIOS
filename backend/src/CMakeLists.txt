add_library(aios_core STATIC
    aios/core/types.cpp
    aios/memory/ProgramLoader.cpp
    aios/cpu/CPU.cpp
    aios/process/ProcessManager.cpp
    aios/interrupt/InterruptManager.cpp
    aios/interrupt/SystemCallManager.cpp
)

target_include_directories(aios_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)