find_package(GTest REQUIRED)
enable_testing()

file(GLOB_RECURSE CPP_TESTS *.cpp)

add_executable(conisTests
    ${CPP_TESTS}
)

# Set output directories for tests
set_target_properties(conisTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_link_libraries(conisTests
    PUBLIC
    ${EXTERNAL_LIBS}
    PRIVATE
    gtest
    gtest_main
    conis::core
)

add_test(NAME conisTests COMMAND conisTests)