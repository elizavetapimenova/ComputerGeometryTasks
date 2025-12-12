project(Viewer2D LANGUAGES CXX)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(Viewer2D
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
)

target_link_libraries(Viewer2D
    PRIVATE
        Qt6::Widgets
        PlaneGeometry
)
