#include <iostream>

#include "ConfigReader/configReader.h"
#include "Logger/logger.h"

int main(int argc, char** argv) {
    std::string configPath = "conf.txt"; //std::string(argv[1]);
    ConfigValues confValues;
    LoggerConfigDescription descr;

    if (ConfigReader::ReadConfig(configPath, descr, confValues) == true) {
        Logger logger;
        logger.Init(confValues);
        logger.Log();
        return 1;
    }
    else {
        std::cout << "Bad config path or data" << std::endl;
        return 0;
    }
}

/*if (argc != 2) {
        std::cout << "Uncorrent count of arguments! Programm need only one argument: path to the logger config!" << std::endl;
        return 0;
    }
    else {
        std::string configPath = std::string(argv[1]);
        ConfigValues confValues;
        if (ConfigReader::ReadConfig(configPath, LoggerConfigDescription(), confValues) == true) {
            Logger lgr(ConfigValues);
            lgr.Log();
            return 1;
        }
        else {
            std::cout << "Bad config path or data" << std::endl;
            return 0;
        }
    } */
