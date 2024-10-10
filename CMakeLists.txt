cmake_minimum_required(VERSION 3.9)

set(CONIC_VERSION_MAJOR 0)
set(CONIC_VERSION_MINOR 3)
set(CONIC_VERSION_PATCH 1)

set(CONIC_VERSION "${CONIC_VERSION_MAJOR}.${CONIC_VERSION_MINOR}.${CONIC_VERSION_PATCH}")

project(conisLauncher VERSION ${CONIC_VERSION} LANGUAGES CXX)

# Set policies and include directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
cmake_policy(SET CMP0079 NEW)

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
file(GLOB_RECURSE CORE_SOURCES src/conis/core/*.cpp src/conis/core/*.hpp)

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

include(GNUInstallDirs)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

option(LIBRARY_ONLY "Only builds the core library" OFF)
if(NOT LIBRARY_ONLY)
    # Add the external libs
    set(CONIC_INCLUDE_DIRS "")
    set(EXTERNAL_LIBS "")
    add_subdirectory(external)

    # Find Qt libraries only for the GUI part of the project
    find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets OpenGL OpenGLWidgets)

    # Define the main executable for the GUI
    file(GLOB_RECURSE CPP_SOURCES src/conis/gui/*.cpp src/conis/main.cpp)
    file(GLOB_RECURSE H_HEADERS src/conis/gui/*.hpp)

    qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${CPP_SOURCES} ${H_HEADERS}
        resources/resources.qrc
    )

    # Specify include directories for the GUI executable
    target_include_directories(${PROJECT_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    )

    # Link the core library and the Qt libraries to the main executable
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
        conis::core
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        ${EXTERNAL_LIBS}
    )

    # Optionally, set properties for the main project executable
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
    )

    # Install the main executable
    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
