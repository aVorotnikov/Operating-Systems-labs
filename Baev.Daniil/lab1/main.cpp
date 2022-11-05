/**
 * @file main.cpp
 * @author Baev Daniil (baev.daniil.2002@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-10-31
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
 * @return int
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