enable_testing()

add_executable(aios_tests
    test_main.cpp
    test_smoke.cpp
)

target_include_directories(aios_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
)

add_test(NAME aios_tests COMMAND aios_tests)