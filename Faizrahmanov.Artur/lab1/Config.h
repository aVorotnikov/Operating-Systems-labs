#pragma once

#include <string>
#include <vector>

class Config
{
private:
    static constexpr size_t SLEEP_TOKEN_COUNT = 1;
    static constexpr size_t PATHS_TOKEN_COUNT = 3;

    struct PathsAndFileExt
    {
        std::string pathFrom;
        std::string pathTo;
        std::string ext;
    };

    std::string configPath;
    static Config* instance; 

    std::vector<PathsAndFileExt> pathsAndFileExt;
    int sleepDuration = 0;

    std::vector<PathsAndFileExt>::iterator curIterator;

    bool configReaded = false;
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

    bool isConfigReaded() const;
private:
    Config();
};