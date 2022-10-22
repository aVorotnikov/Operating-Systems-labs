#pragma once

#include "configDescription.h"

// Namespace, which contains functions to read config
namespace ConfigReader {
    // Method which read config file data
    bool ReadConfig(const std::string& configPath, const ConfigDescription& descr, ConfigValues& values);
};
