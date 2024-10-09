# conis/core/CMakeLists.txt

cmake_minimum_required(VERSION 3.9)

set(CONIC_VERSION_MAJOR 0)
set(CONIC_VERSION_MINOR 3)
set(CONIC_VERSION_PATCH 1)

set(CONIC_VERSION "${CONIC_VERSION_MAJOR}.${CONIC_VERSION_MINOR}.${CONIC_VERSION_PATCH}")

project(core VERSION ${CONIC_VERSION} LANGUAGES CXX)

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
file(GLOB_RECURSE CORE_SOURCES *.cpp *.hpp)

# Define the core library
add_library(conis_core STATIC ${CORE_SOURCES})

# Set output directories for the core library
set_target_properties(conis_core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" # For static libs
)

# Set include directories for core library
target_include_directories(conis_core
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..> # Include the parent directory for conis/
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
