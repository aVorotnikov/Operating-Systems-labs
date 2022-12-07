#include <iostream>
#include "daemon/Daemon.hpp"
#include <filesystem>

int main(int argc, char **argv){
    try {
        if(argc != 2){
            std::cout << "Not enough arguments" << std::endl;
            return -1;
        }
        Daemon::getInstance().init(argv[1]);
        Daemon::getInstance().run();
    } catch (std::exception const& e){
        syslog(LOG_ERR, "ERROR:: %s", e.what());
        Daemon::getInstance().terminate();
    }
    return 0;
}