set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_library(gui
    src/gui.cpp
    gui.h
    ${GUI_SRC}
    ${GUI_UI}
)
target_link_libraries(gui
    Qt6::Widgets
)