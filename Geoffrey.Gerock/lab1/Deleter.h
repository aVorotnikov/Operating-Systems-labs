#pragma once

#include "ConfigReader.h"

class Deleter {
public:
    static void deleteDirs(const std::vector<ConfigItem>& items);

private:
    static void _deleteItem(const ConfigItem& item);

    static void _traverseTree(const fs::path& currPath, const int currDepth, const int maxDepth);

    static const int _rootDepth = 0;
};
