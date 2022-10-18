#pragma once

#include <string>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>

class FileMoverDaemon
{
private:
    const std::string PID_PATH = "/var/run/fileMover.pid";

    bool isRunning;

public:
    ~FileMoverDaemon() = default;

    static FileMoverDaemon& getInstance();

    void initialize(const std::string &configPath);

    void run();
    void stop();

private:
    FileMoverDaemon() = default;

    FileMoverDaemon(FileMoverDaemon &other) = delete;
    void operator=(const FileMoverDaemon &other) = delete;

    void createPid();
    void moveFiles();
    void removeFiles(const std::string &path);

    void destructOldPid();

    bool pathExist(const std::string &path) const;
};