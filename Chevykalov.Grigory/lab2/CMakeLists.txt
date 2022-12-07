cmake_minimum_required(VERSION 3.10)
project(lab2_multiprocess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_library (LIBRT rt)
find_package (Threads)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

file(GLOB CONNS connection/conn_*.h)
file(GLOB CLIENT client/*.cpp client/*.h)
file(GLOB GUI gui/*.ui gui/*.cpp gui/*.h )
file(GLOB HOST host/*.cpp host/*.h)

qt_wrap_cpp(GUI_SRC gui/gui.h)
qt_wrap_ui(GUI_UI gui/MainWindow.ui)
foreach(PATH_TO_CONN ${CONNS})
    get_filename_component(FILE_NAME ${PATH_TO_CONN} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_NAME ${FILE_NAME})

    add_executable(client_${CONN_NAME} ${CLIENT}
            connection/connection.h connection/conn_${CONN_NAME}.cpp connection/conn_${CONN_NAME}.h)
    add_executable(host_${CONN_NAME} ${GUI} ${GUI_UI} ${GUI_SRC} ${HOST}
            connection/connection.h connection/conn_${CONN_NAME}.cpp connection/conn_${CONN_NAME}.h)
    
    target_link_libraries(client_${CONN_NAME} PUBLIC pthread)     
    target_link_libraries(host_${CONN_NAME} PUBLIC pthread rt Qt6::Widgets)
endforeach()