cmake_minimum_required(VERSION 3.16)
project(lab2)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror)

find_package (Threads)
find_library (LIBRT rt)

file(GLOB CONNECTIONS connections/conn_*.h)
file(GLOB HOST host/*.cpp host/*.h)
file(GLOB COMMON common/*.h)
file(GLOB CLIENT client/*.cpp client/*.h)

foreach(PATH_TO_CON ${CONNECTIONS})
    get_filename_component(FILE_NAME ${PATH_TO_CON} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_NAME ${FILE_NAME})

    add_executable(host_${CONN_NAME} ${COMMON} ${HOST}
            connections/connection.h connections/conn_${CONN_NAME}.cpp connections/conn_${CONN_NAME}.h)
    add_executable(client_${CONN_NAME} ${COMMON} ${CLIENT}
            connections/connection.h connections/conn_${CONN_NAME}.cpp connections/conn_${CONN_NAME}.h host/host.h)
    target_link_libraries(host_${CONN_NAME} PUBLIC pthread rt)
    target_link_libraries(client_${CONN_NAME} PUBLIC pthread rt)
endforeach()
