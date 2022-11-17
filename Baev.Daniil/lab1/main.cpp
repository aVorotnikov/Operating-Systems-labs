/**
 * @file main.cpp
 * @author Baev Daniil (baev.daniil.2002@gmail.com)
 * @brief 
 * @version 0.2
 * @date 2022-11-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <iostream>
#include "daemon.hpp"

/**
 * @brief main function
 * 
 * @param argc 
 * @param argv 
 * @return int exit code
 */
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Incorrect arguments. Provide only one argument - configuration file." << std::endl;
        return EXIT_FAILURE;
    }
    
    Daemon::getInstance().init(argv[1]);
    Daemon::getInstance().run();
    
    return EXIT_SUCCESS;
}