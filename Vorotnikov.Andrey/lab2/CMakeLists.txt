cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

project("lab2")

add_subdirectory(connections)

file(GLOB CONN_FILES connections/conn_*.h)
foreach(CONN_FILE ${CONN_FILES})
    get_filename_component(FILE_NAME ${CONN_FILE} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_TYPE ${FILE_NAME})

    add_executable(host_${CONN_TYPE}
        host/main.cpp
    )
    target_link_libraries(host_${CONN_TYPE}
        connection_${CONN_TYPE}
    )

    add_executable(client_${CONN_TYPE}
        client/main.cpp
    )
    target_link_libraries(client_${CONN_TYPE}
        connection_${CONN_TYPE}
    )
endforeach()
