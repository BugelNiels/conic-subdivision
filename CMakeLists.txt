cmake_minimum_required(VERSION 3.16)
project(ConicSubdiv VERSION 1.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui OpenGL OpenGLWidgets Widgets REQUIRED)
find_package (Eigen3 3.3 REQUIRED)

include_directories(src)

file(GLOB_RECURSE CPP_SOURCES src/*.cpp)
file(GLOB_RECURSE H_HEADERS src/*.hpp)

qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${CPP_SOURCES} ${H_HEADERS}
        resources/resources.qrc
        src/util/objcurvereader.cpp src/util/objcurvereader.hpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        Qt6::Widgets
        Eigen3::Eigen
        )

install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
