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

    std::vector<PathsAndFileExt> pathsAndFileExt;
    unsigned sleepDuration = 0;

    std::vector<PathsAndFileExt>::iterator curIterator;

    bool configReaded = false;
public:
    ~Config() = default;

    static Config& getInstance();

    void setConfigPath(const std::string& configPath);

    bool readConfig();

    bool next();

    std::string getFromPath() const;
    std::string getToPath() const;
    std::string getFileExt() const;
    unsigned getSleepDuration() const;

    bool isConfigReaded() const;
private:
    Config() = default;
    Config(Config& other) = delete;
    void operator=(const Config& other) = delete;
};