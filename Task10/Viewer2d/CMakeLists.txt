cmake_minimum_required(VERSION 3.16)
project(Viewer2d LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Widgets REQUIRED)
qt_standard_project_setup()

qt_add_executable(PolyIntersectionArbitraryViewer
    main.cpp
    MainWindow.h
    MainWindow.cpp
    CanvasWidget.h
    CanvasWidget.cpp
    MainWindow.ui
)

target_link_libraries(PolyIntersectionArbitraryViewer PRIVATE Qt6::Widgets PlaneGeometry)
qt_finalize_executable(PolyIntersectionArbitraryViewer)
