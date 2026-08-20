enable_testing()

add_executable(aios_tests
    test_main.cpp
    test_smoke.cpp
    test_types.cpp
    test_memory.cpp
    test_loader.cpp
    test_cpu.cpp
    test_process.cpp
    test_interrupt.cpp
    test_syscall.cpp
    test_paging.cpp
    test_timer.cpp
)

target_include_directories(aios_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
)

target_link_libraries(aios_tests PRIVATE aios_core)

add_test(NAME aios_tests COMMAND aios_tests)