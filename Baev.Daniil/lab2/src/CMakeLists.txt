cmake_minimum_required(VERSION 3.14)

project(WolfAndGoats VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Werror)

find_library (LIBRT rt)
find_package (Threads)
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

file(GLOB CONNS connection/conn_*.cpp)
file(GLOB CLIENT client/*.cpp client/*.h)
file(GLOB GUI gui/*.ui gui/*.cpp gui/*.h )
file(GLOB HOST host/*.cpp host/*.h)

#qt_wrap_cpp(GUI_SRC gui/gui.h)
#qt_wrap_ui(GUI_UI gui/MainWindow.ui)
message("--Gernerate ui_ ${GUI_UI}")
foreach(PATH_TO_CONN ${CONNS})
    get_filename_component(FILE_NAME ${PATH_TO_CONN} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_NAME ${FILE_NAME})
    
    add_executable(host_${CONN_NAME} ${GUI} ${GUI_UI} ${GUI_SRC} ${HOST} ${CLIENT}
    connection/connection.h connection/conn_${CONN_NAME}.cpp)
       
    target_link_libraries(host_${CONN_NAME} PUBLIC pthread rt Qt6::Widgets)
endforeach()