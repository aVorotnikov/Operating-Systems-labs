#include "daemon/daemon.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Invalid input arguments" << std::endl;
        return EXIT_FAILURE;
    }
    auto& daemon = Daemon::getInstance();
    daemon.initialize(argv[1]);
    daemon.run();
    return EXIT_SUCCESS;
}