#include <iostream>
#include <fstream>
#include "FileMoverDaemon.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Incorrect arguments" << std::endl;
        return EXIT_FAILURE;
    }
    FileMoverDaemon::getInstance()->initialize(argv[1]);
    FileMoverDaemon::getInstance()->run();

    return EXIT_SUCCESS;
}