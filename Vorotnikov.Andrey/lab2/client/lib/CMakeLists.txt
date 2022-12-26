find_library(LIBRT rt)
find_package(Threads)
add_library(client_lib
    src/client.cpp
)
target_link_libraries(client_lib
    game_rules
    pthread
    rt
)