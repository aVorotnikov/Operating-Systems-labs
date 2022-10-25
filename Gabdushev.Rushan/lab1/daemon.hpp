#pragma once

#include <string>
#include <chrono>

class DaemonTmpCleaner
{
private:
    const std::string pidPath = "/var/run/daemon_tmp_cleaner.pid";
    const std::string targetFileExtention = ".tmp";
    const std::chrono::seconds defaultSleepTime = std::chrono::seconds(10);

    std::string configAbsPath;
    std::string targetDirPath;
    std::chrono::seconds sleepTime;
    bool isTerminateReceived = false;

    DaemonTmpCleaner(){};
    DaemonTmpCleaner(const DaemonTmpCleaner &) = delete;
    DaemonTmpCleaner &operator=(const DaemonTmpCleaner &) = delete;

    void WritePid();
    void Daemonize();
    void KillExistingDaemon();

public:
    static DaemonTmpCleaner &Get()
    {
        static DaemonTmpCleaner instance;
        return instance;
    }

    void Initialize(std::string configLocalPath);

    // SIGTERM handler
    void Terminate();

    // SIGHUP handler
    void ReloadConfig();

    void Run();
};