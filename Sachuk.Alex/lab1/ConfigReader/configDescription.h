#pragma once

#include <vector>
#include <queue>
#include <string>

// Abstract class with config data description
class ConfigDescription {
protected:
    uint totalLines;
    std::vector<uint> strLinesIds;
    std::vector<uint> uintLinesIds;

    ConfigDescription() = default;
public:
    uint GetTotalLinesCnt() const { return totalLines; };
    std::vector<uint> GetStrLinesIds() const { return strLinesIds; };
    std::vector<uint> GetUintLinesIds() const { return uintLinesIds; };
};

// Struct which contains all readed data from configs
struct ConfigValues {
    std::queue<std::string> strData;
    std::queue<uint> uintData;
};
