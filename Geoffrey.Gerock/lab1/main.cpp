#include <iostream>

#include "Daemon.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "No config was passed";
        return EXIT_FAILURE;
    }
    // resolve the path before changing to `/`
    auto configPath = fs::current_path() / argv[1];
    Daemon::init();
    Daemon::getInstance().setConfigPath(configPath);
    Daemon::readConfig();
    Daemon::getInstance().run();

    return EXIT_SUCCESS;
}
