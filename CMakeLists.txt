cmake_minimum_required(VERSION 3.9)

project(conisLauncher LANGUAGES CXX)

# Set policies and include directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
cmake_policy(SET CMP0079 NEW)


add_subdirectory(conis/core)

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
    file(GLOB_RECURSE CPP_SOURCES conis/gui/*.cpp conis/main.cpp)
    file(GLOB_RECURSE H_HEADERS conis/gui/*.hpp)

    qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${CPP_SOURCES} ${H_HEADERS}
        resources/resources.qrc
    )

    # Specify include directories for the GUI executable
    target_include_directories(${PROJECT_NAME}
        PRIVATE
        conis/gui   # Include GUI directory for main executable
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

