#include <iostream>

#include "Daemon/daemon.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Uncorrent count of arguments! Programm need only one argument: path to the logger config!" << std::endl;
        return EXIT_FAILURE;
    }
    else {
        std::string configPath = std::string(argv[1]);
        Daemon::GetDaemonInstance().Init(configPath);
        Daemon::GetDaemonInstance().Run();
        return EXIT_SUCCESS;
    }
}
