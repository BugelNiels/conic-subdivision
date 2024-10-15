# Find and include Eigen3 for the core library
find_package(Eigen3 3.3 REQUIRED)
find_package(OpenMP REQUIRED)

set(CONIC_LIBS "")
list(APPEND CONIC_LIBS Eigen3::Eigen)

if(OpenMP_FOUND)
    list(APPEND CONIC_LIBS OpenMP::OpenMP_CXX)
else()
    message("OpenMP was not found.")
endif()

# Collect source files for core library
file(GLOB_RECURSE CORE_SOURCES src/*.cpp src/*.hpp include/*.hpp)

# Define the core library
add_library(conis_core STATIC ${CORE_SOURCES})

# Add compile options to the core library
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC or Clang compiler: enable FMA and target the native architecture
    target_compile_options(conis_core PRIVATE -mfma -march=native)
endif()

# Set output directories for the core library
set_target_properties(conis_core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Set include directories for core library
target_include_directories(conis_core
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Link Eigen and OpenMP libraries to core
target_link_libraries(conis_core
    PUBLIC
    ${CONIC_LIBS}
)

# Set properties such as C++ standard for core library
set_target_properties(conis_core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

# Create a namespaced alias for the core library
add_library(conis::core ALIAS conis_core)

# Testing
if(BUILD_UNIT_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()

    file(GLOB_RECURSE CPP_TESTS test/*.cpp)

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
endif()
