#include "daemon.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    std::string config_path;

    if (argc == 1)
        config_path = "config.txt";
    else if (argc == 2)
        config_path = argv[1];
    else {
        std::cout << "Unsupported amount of arguments" << std::endl;
        std::cout << "Must be either empty or a single string containing path to config file" << std::endl;
        return 1;
    }

    Daemon &daemon = Daemon::create(config_path);
    daemon.start();
}