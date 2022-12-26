add_library(host_lib
    src/host.cpp
)
target_link_libraries(host_lib
    gui
    game_rules
)
