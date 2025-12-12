add_library(DelaunayCore
  src/delaunay.cpp
)

target_include_directories(DelaunayCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
