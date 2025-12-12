find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(DelaunayMinimalViewer
  src/main.cpp
  src/Canvas.h src/Canvas.cpp
)

target_link_libraries(DelaunayMinimalViewer PRIVATE Qt6::Widgets DelaunayCore)
