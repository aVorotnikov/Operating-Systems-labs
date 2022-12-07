cmake_minimum_required(VERSION 3.14)

project(WolfAndGoats VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Werror -DCMAKE_EXE_LINKER_FLAGS=-latomic)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick REQUIRED)
find_package (Threads)
find_library (LIBRT rt)

set(CLIENT goat)
set(HOST wolf)
set(CONNECTION connections)
set(UTILS utils)

foreach(CONN mq fifo sock)
    add_executable(host_${CONN} ${HOST}/wolf.cpp ${HOST}/wolf.h ${HOST}/host.cpp
            ${CLIENT}/goat.cpp ${CLIENT}/goat.h
            ${UTILS}/configuration.h ${UTILS}/message.h
            ${CONNECTION}/conn.h ${CONNECTION}/conn_${CONN}.cpp ${CONNECTION}/conn_${CONN}.h
            gui/gui.cpp gui/gui.h
            gui/qml.qrc)
    target_link_libraries(host_${CONN} PUBLIC rt pthread Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick)
endforeach(CONN mq fifo sock)
