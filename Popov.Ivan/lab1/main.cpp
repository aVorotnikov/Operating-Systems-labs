#include "daemon.h"
#include <iostream>

int main(int argc, char** argv)
 {
    if (argc != 2) {
        std::cout << "Invalid input arguments" << std::endl;
        return EXIT_FAILURE;
    }
    
    Daemon& daemon = Daemon::GetInstance();
    daemon.Initialize(argv[1]);
    daemon.Run();
    return EXIT_SUCCESS;
}
