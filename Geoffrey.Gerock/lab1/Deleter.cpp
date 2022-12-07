#include <sys/syslog.h>
#include <numeric>
#include <algorithm>

#include "Deleter.h"

void Deleter::deleteDirs(const std::vector<ConfigItem>& items) {
    std::for_each(items.cbegin(), items.cend(), Deleter::_deleteItem);
}

void Deleter::_deleteItem(const ConfigItem& item) {
    auto [localRoot, depth] = item;
    if (!exists(localRoot)) {
        syslog(LOG_INFO, "There is no such filepath or it was already removed (%s)", localRoot.c_str());
        return;
    }
    auto it = fs::directory_iterator(localRoot);
    // save the dirs and files at `rootDepth + 1` depth to delete only them,
    // not the moved from the further depth
    std::vector<fs::directory_entry> toRemove(begin(it), end(it));
    try {
        _traverseTree(localRoot, _rootDepth + 1, depth);
        std::for_each(toRemove.begin(), toRemove.end(), [](const auto& entry) { fs::remove_all(entry.path()); });
    } catch (fs::filesystem_error& err) {
        syslog(LOG_ERR, "%s", err.what());
    }
}

void Deleter::_traverseTree(const fs::path& currPath, const int currDepth, const int maxDepth) {
    auto it = fs::directory_iterator(currPath);

    // these files should not be removed, move them under the `currRoot`
    if (currDepth > maxDepth) {
        for (const auto& entry: it) {
            const auto& path = entry.path();
            auto part = path.end();
            // get the last part of the residual path into `part`
            for (int depth = currDepth + 1; depth > _rootDepth + 1; part--, depth--);
            // collect the path from its parts, add the filename of the current entry
            fs::rename(path, std::accumulate(std::next(path.begin()), part, fs::path{}, std::divides{}) /
                             path.filename());
        }
    } else
        for (const auto& entry: it)
            if (entry.is_directory())
                _traverseTree(entry.path(), currDepth + 1, maxDepth);
}
