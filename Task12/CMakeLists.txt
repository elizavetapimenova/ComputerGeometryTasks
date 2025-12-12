cmake_minimum_required(VERSION 3.16)

project(Task12 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(PlaneGeometry)
add_subdirectory(Viewer2D)
