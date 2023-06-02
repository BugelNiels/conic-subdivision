cmake_minimum_required(VERSION 3.16)
project(ConicSubdiv VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS OpenGL OpenGLWidgets Widgets)
find_package (Eigen3 3.3 REQUIRED)

include_directories(src)

file(GLOB_RECURSE CPP_SOURCES src/*.cpp)
file(GLOB_RECURSE H_HEADERS src/*.hpp)

qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${CPP_SOURCES} ${H_HEADERS}
        resources/resources.qrc
        src/util/vector.hpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt::Core
        Qt::Gui
        Eigen3::Eigen
        )

if ((QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Qt::OpenGL
            Qt::OpenGLWidgets
            Qt::Widgets
            )
endif ()

install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
