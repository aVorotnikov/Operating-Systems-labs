add_library(client_lib
    src/client.cpp
)
target_link_libraries(client_lib
    game_rules
)