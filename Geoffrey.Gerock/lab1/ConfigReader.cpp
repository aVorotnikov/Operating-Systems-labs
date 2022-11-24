#include <fstream>
#include <sys/syslog.h>

#include "ConfigReader.h"
#include "utils.h"

int ConfigReader::readConfig() {
    _items.clear();
    if (!_isInited) {
        syslog(LOG_CRIT, "No config path was set");
        return EXIT_FAILURE;
    }

    std::ifstream configFile(_configPath.c_str());
    if (!configFile.is_open()) {
        syslog(LOG_CRIT, "Couldn't open config file");
        return EXIT_FAILURE;
    }
    int depth;
    size_t numIdx;
    for (std::string line; std::getline(configFile, line);) {
        trim(line);
        if ((numIdx = line.rfind(' ')) == std::string::npos) {
            syslog(LOG_ERR, "Incorrect config line (%s)", line.c_str());
            continue;
        }
        depth = std::stoi(line.substr(numIdx));
        if (depth < _rootDepth + 1) {
            syslog(LOG_INFO, "Depth is less than %d, nothing to delete -> skipping", _rootDepth + 1);
            continue;
        }
        _items.emplace_back(std::make_pair<>(fs::path(line.substr(0, numIdx)), depth));
    }
    return EXIT_SUCCESS;
}

std::vector<ConfigItem>& ConfigReader::getItems() {
    return _items;
}
