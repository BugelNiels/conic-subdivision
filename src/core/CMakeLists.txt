find_package(Eigen3 3.3 REQUIRED)
find_package(OpenMP REQUIRED)

set(CONIC_LIBS "")
list(APPEND CONIC_LIBS Eigen3::Eigen)

if(OpenMP_FOUND)
    list(APPEND CONIC_LIBS OpenMP::OpenMP_CXX)
else()
    message("OpenMP was not found.")
endif()

file(GLOB_RECURSE CORE_SOURCES src/*.cpp src/*.hpp include/*.hpp)

add_library(conis_core STATIC ${CORE_SOURCES}
        test/test_helpers.cpp
        test/test_helpers.hpp)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC or Clang compiler: enable FMA and target the native architecture
    target_compile_options(conis_core PRIVATE -mfma -march=native)
endif()

set_target_properties(conis_core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(conis_core
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(conis_core
    PUBLIC
    ${CONIC_LIBS}
)

set_target_properties(conis_core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

add_library(conis::core ALIAS conis_core)

# Testing
option(BUILD_UNIT_TESTS "Builds the unit test" ON)
if(BUILD_UNIT_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()

    file(GLOB_RECURSE CPP_TESTS test/*.cpp)

    add_executable(conisTests
        ${CPP_TESTS}
    )

    set_target_properties(conisTests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    target_link_libraries(conisTests
        PRIVATE
        gtest
        gtest_main
        conis::core
    )

    add_test(NAME conisTests COMMAND conisTests)
endif()
