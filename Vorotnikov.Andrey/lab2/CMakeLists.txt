cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

project("wolf_and_goats")

add_subdirectory(connections)
add_subdirectory(host)
add_subdirectory(client)

file(GLOB CONN_FILES connections/conn_*.h)
foreach(CONN_FILE ${CONN_FILES})
    get_filename_component(FILE_NAME ${CONN_FILE} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_TYPE ${FILE_NAME})

    add_executable(host_${CONN_TYPE}
        host/src/main.cpp
    )
    target_link_libraries(host_${CONN_TYPE}
        connection_${CONN_TYPE}
        host_lib
    )

    add_executable(client_${CONN_TYPE}
        client/src/main.cpp
    )
    target_link_libraries(client_${CONN_TYPE}
        connection_${CONN_TYPE}
        client_lib
    )
endforeach()
