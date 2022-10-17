#include <fstream>
#include <algorithm>

#include "configReader.h"

// Namespace, which contains functions to read config
namespace ConfigReader {
    // inline func
    inline bool IsInVector(const std::vector<uint>& vec, uint item) {
        return std::find(vec.begin(), vec.end(), item) != vec.end();
    }

    // Method which read config file data
    bool ReadConfig(const std::string& configPath, const AbstractConfigDescription& descr, ConfigValues& values) {
        // Clear old data
        values.strData = std::queue<std::string>();
        values.uintData = std::queue<uint>();

        // Open config to read
        std::string line;
        uint currentLineIndx = 0;
        std::ifstream configStream(configPath);            

        if (!configStream.is_open())
            return false;

        // Config description
        uint totalLines = descr.GetTotalLinesCnt();
        std::vector<uint> strLinesIndxs = descr.GetStrLinesIds();
        std::vector<uint> uintLinesIndxs = descr.GetUintLinesIds();

        // Read lines
        while (std::getline(configStream, line) && totalLines > currentLineIndx) {
            // If empty line -> skip
            if (line.empty())
                continue;
            else {
                // If 'str' data
                if (IsInVector(strLinesIndxs, currentLineIndx)) {
                    values.strData.push(line);
                    currentLineIndx++;
                }
                // If 'uint' data
                else if (IsInVector(uintLinesIndxs, currentLineIndx)) {
                    values.uintData.push(std::stoul(line));
                    currentLineIndx++;
                }
                // If smthng unknown -> leave
                else
                    return false;
            }
        }

        configStream.close();

        // Make last check of readed data
        if (descr.GetUintLinesIds().size() == values.uintData.size() &&
            descr.GetStrLinesIds().size() == values.strData.size())
            return true;
        else
            return false;
    }
};
