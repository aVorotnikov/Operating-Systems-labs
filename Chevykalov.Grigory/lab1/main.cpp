#include "daemon.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Incorrect arguments. Provide only one argument - configuration file." << std::endl;
        return EXIT_FAILURE;
    }
    
    DaemonReminder::GetInstance().Init(argv[1]);
    DaemonReminder::GetInstance().Run();
    
    return EXIT_SUCCESS;
}