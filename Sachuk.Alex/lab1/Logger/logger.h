#pragma once

#include "../ConfigReader/abstractConfigDescription.h"

// Class, which describes 'Logger' config data
class LoggerConfigDescription : public AbstractConfigDescription {
public:
    LoggerConfigDescription() {
        totalLines = LOGGER_CONF_TOTAL_LINES;
        strLinesIds = LOGGER_CONF_STR_LINES_INDXS;
        uintLinesIds = LOGGER_CONF_UINT_LINES_INDXS;
    }
private:
    // Total conf lines
    static constexpr uint LOGGER_CONF_TOTAL_LINES = 3;                      
    // Lines indexes with 'string' type data
    std::vector<uint> LOGGER_CONF_STR_LINES_INDXS = {0, 1};
    // Lines indexes with 'uint' type data
    std::vector<uint> LOGGER_CONF_UINT_LINES_INDXS = {2};
};


// Class which provides files logging
class Logger {
public:
    // Delete all standart constructors
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;

    // Constructor, which init fields with data from config 
    void Init(ConfigValues& values);

    // Method, which provides logging
    bool Log(void);

    // Method, which returns refresh duration
    uint GetRefreshDuration() { return refreshDuration; };
    std::string GetFirstDir() { return firstDirPath; };
    std::string GetSecondDir() { return secondDirPath; };
private:
    // Method, which provides executing directory 
    bool GetAllDirsAndFiles(const std::string& directoryPath, std::queue<std::string>& dirsAndFiles);

    // Method, which generates log string
    std::string GenerateLogString(std::queue<std::string>& dirsAndFiles);

    // File name with log
    std::string LOGGER_FILE_NAME = "hist.log";

    std::string firstDirPath;       // Path of first directory (which need to be logged)
    std::string secondDirPath;      // Path of second directory (where placed logger)
    uint refreshDuration;           // Log refresh duration
};
