#pragma once

#include <string>
#include <vector>

class Config
{
    using PathFromAndTo = std::pair<std::string, std::string>;
    using PathsAndFileExt = std::pair<PathFromAndTo, std::string>;
private:
    std::string configPath;
    static Config* instance; 

    std::vector<PathsAndFileExt> pathsAndFileExt;
    int sleepDuration = 0;

    std::vector<PathsAndFileExt>::iterator curIterator;
public:
    Config(Config& other) = delete;
    void operator=(const Config& other) = delete;

    ~Config();

    static Config* getInstance();

    void setConfigPath(const std::string& configPath);

    bool readConfig();

    bool next();

    std::string getFromPath() const;
    std::string getToPath() const;
    std::string getFileExt() const;
    int getSleepDuration() const;
private:
    Config();
};