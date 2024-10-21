include(GNUInstallDirs)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CONIC_INCLUDE_DIRS "")
set(EXTERNAL_LIBS "")
add_subdirectory(${PROJECT_SOURCE_DIR}/external ${CMAKE_BINARY_DIR}/external)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets OpenGL OpenGLWidgets)

file(GLOB_RECURSE CPP_SOURCES src/*.cpp)
file(GLOB_RECURSE H_HEADERS src/*.hpp)

qt_add_executable(conisLauncher WIN32 MACOSX_BUNDLE
    ${CPP_SOURCES} ${H_HEADERS}
    resources/resources.qrc
)

target_include_directories(conisLauncher
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    ${CONIC_INCLUDE_DIRS}
)

target_link_libraries(conisLauncher
    PUBLIC
    conis::core
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    ${EXTERNAL_LIBS}
)

set_target_properties(conisLauncher PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

install(TARGETS conisLauncher
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)