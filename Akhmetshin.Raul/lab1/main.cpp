#include "daemon.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Incorrect list of arguments. Provide only one argument - configuration file." << std::endl;
        return EXIT_FAILURE;
    }

    DaemonFileRemover::GetInstance().Init(argv[1]);
    DaemonFileRemover::GetInstance().Run();
    
    return EXIT_SUCCESS;
}