cmake_minimum_required(VERSION 3.1.0)

project(wolf_n_goats VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Werror)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

find_package (Threads)
find_library (LIBRT rt)


file(GLOB CONN_FILES connections/conn_*.h)
foreach(CONN_FILE ${CONN_FILES})
    get_filename_component(FILE_NAME ${CONN_FILE} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN ${FILE_NAME})

    add_executable(host_${CONN}
        host/host.h
        host/host.cpp
        host/client_handler.h
        host/client_handler.cpp
        host/wolf_window/gui_form.ui
        host/wolf_window/gui_window.cpp
        client/client.h
        client/client.cpp
        connections/conn.h
        connections/conn_${CONN}.cpp connections/conn_${CONN}.h)

    target_link_libraries(host_${CONN} PUBLIC pthread rt Qt${QT_VERSION_MAJOR}::Widgets)
endforeach()