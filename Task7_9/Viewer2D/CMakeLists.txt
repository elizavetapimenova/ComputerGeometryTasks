cmake_minimum_required(VERSION 3.16)
project(Viewer2D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Widgets REQUIRED)
qt_standard_project_setup()

add_executable(Viewer2D
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/DrawingWidget.cpp
    src/DrawingWidget.h
    src/MainWindow.ui
)

target_link_libraries(Viewer2D PRIVATE Qt6::Widgets PlaneGeometry)
qt_finalize_executable(Viewer2D)
