#include "Config.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

Config *Config::instance = nullptr;

Config::Config()
{
}

Config::~Config()
{
    delete instance;
}

Config *Config::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Config();
    }

    return instance;
}

void Config::setConfigPath(const std::string &configPath)
{
    this->configPath = std::filesystem::absolute(configPath);
}

bool Config::readConfig()
{
    sleepDuration = 0;
    pathsAndFileExt.clear();
    configReaded = false;

    std::ifstream file;

    file.open(configPath);

    if (!file)
        return false;

    try
    {
        std::string line;
        while (std::getline(file, line))
        {
            std::stringstream ss(line);

            std::vector<std::string> tokens;
            std::string token;
            while (std::getline(ss, token, ' '))
            {
                tokens.push_back(token);
            }

            PathsAndFileExt tmp;
            int tokenCount = tokens.size();
            if (tokenCount == 1)
            {
                sleepDuration = std::stoi(tokens.at(0));
            }
            else if (tokenCount == 3)
            {
                tmp.pathFrom = tokens[0];
                tmp.pathTo = tokens[1];
                tmp.ext = tokens[2];
                pathsAndFileExt.push_back(tmp);
            }
            else
            {
                return false;
            }

        }
    }
    catch (...)
    {
        return false;
    }

    if (pathsAndFileExt.size() == 0)
        return false;

    curIterator = pathsAndFileExt.begin();
    configReaded = true;

    return true;
}

bool Config::next()
{
    curIterator++;
    if (curIterator == pathsAndFileExt.end())
    {
        curIterator = pathsAndFileExt.begin();
        return false;
    }

    return true;
}

std::string Config::getFromPath() const
{
    return (*curIterator).pathFrom;
}

std::string Config::getToPath() const
{
    return (*curIterator).pathTo;
}

std::string Config::getFileExt() const
{
    return (*curIterator).ext;
}

int Config::getSleepDuration() const
{
    return sleepDuration;
}

bool Config::isConfigReaded() const
{
    return configReaded;
}