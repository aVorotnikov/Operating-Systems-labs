file(GLOB CONN_FILES conn_*.h)
foreach(CONN_FILE ${CONN_FILES})
    get_filename_component(FILE_NAME ${CONN_FILE} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_TYPE ${FILE_NAME})

    add_library(connection_${CONN_TYPE}
        src/conn.cpp
        src/conn_${CONN_TYPE}.cpp
    )
endforeach()
