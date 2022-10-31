#pragma once

#include <utility>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;
typedef std::pair<const fs::path, const int> ConfigItem;

class ConfigReader {

public:
    void setPath(const fs::path& configPath) {
        const_cast<fs::path&>(_configPath) = configPath;
        _isInited = true;
    }

    bool isInited() const { return _isInited; }

    int readConfig();

    std::vector<ConfigItem>& getItems();

private:
    const fs::path _configPath;
    bool _isInited = false;
    std::vector<ConfigItem> _items;
    static inline const int _rootDepth = 0;
};
