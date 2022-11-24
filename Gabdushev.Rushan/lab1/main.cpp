#include "daemon.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Incorrect list of arguments. Provide only one argument - configuration file." << std::endl;
        return EXIT_FAILURE;
    }
    
    DaemonTmpCleaner::Get().Initialize(argv[1]);
    DaemonTmpCleaner::Get().Run();
    
    return EXIT_SUCCESS;
}