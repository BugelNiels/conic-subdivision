cmake_minimum_required(VERSION 3.9)

set(CONIC_VERSION_MAJOR 0)
set(CONIC_VERSION_MINOR 2)
set(CONIC_VERSION_PATCH 1)

set(CONIC_VERSION "${CONIC_VERSION_MAJOR}.${CONIC_VERSION_MINOR}.${CONIC_VERSION_PATCH}")
set(CONIC_NAME "conicsubdiv")

project(conicsubdiv VERSION ${CONIC_VERSION} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
cmake_policy(SET CMP0079 NEW)

include(GNUInstallDirs)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets OpenGL OpenGLWidgets)
find_package(OpenMP REQUIRED)
find_package(Eigen3 3.3 REQUIRED)

set(CONIC_INCLUDE_DIRS "")
set(EXTERNAL_LIBS "")
add_subdirectory(external)

set(CONIC_LIBS "")
list(APPEND CONIC_LIBS Qt6::Core)
list(APPEND CONIC_LIBS Qt6::Gui)
list(APPEND CONIC_LIBS Qt6::Widgets)
list(APPEND CONIC_LIBS Qt6::OpenGL)
list(APPEND CONIC_LIBS Qt6::OpenGLWidgets)
list(APPEND CONIC_LIBS Eigen3::Eigen)
if (OpenMP_FOUND)
    list(APPEND CONIC_LIBS OpenMP::OpenMP_CXX)
else ()
    message("OpenMP was not found.")
endif ()

include_directories(
        src
        ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE CPP_SOURCES src/*.cpp)
file(GLOB_RECURSE H_HEADERS src/*.hpp)

qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${CPP_SOURCES} ${H_HEADERS}
        resources/resources.qrc)

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CONIC_INCLUDE_DIRS}
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ${EXTERNAL_LIBS}
        PRIVATE
        ${CONIC_LIBS}
        )

install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
