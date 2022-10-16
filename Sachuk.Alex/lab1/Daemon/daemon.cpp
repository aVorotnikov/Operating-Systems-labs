#include <unistd.h>

#include "daemon.h"

// Default initializer method
bool Daemon::Init(const std::string& localConfigPath) {
    // load and save global config path
    char workingDirPath[300];
    getcwd(workingDirPath, sizeof(workingDirPath));
    globalConfigPath = std::string(workingDirPath) + "/" + localConfigPath;
}