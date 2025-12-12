cmake_minimum_required(VERSION 3.16)
project(PlaneGeometry LANGUAGES CXX)

add_library(PlaneGeometry STATIC
    src/Geometry.cpp
)
target_include_directories(PlaneGeometry
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
