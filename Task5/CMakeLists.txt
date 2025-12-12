cmake_minimum_required(VERSION 3.16)
project(ComputerGeometryTask5 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

add_executable(ComputerGeometryTask5
    main.cpp
    mainwindow.cpp
    mainwindow.h
    algorithms/convex_hull.cpp
    algorithms/convex_hull.hpp
)

target_link_libraries(ComputerGeometryTask5 PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

include(GNUInstallDirs)
install(TARGETS ComputerGeometryTask5
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ComputerGeometryTask5)
endif()
