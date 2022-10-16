#pragma once

#include <string>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>

class FileMoverDaemon
{
private:
    const std::string PID_PATH = "/var/run/fileMover.pid";

    static FileMoverDaemon* instance;

    bool isRunning;
public:
    FileMoverDaemon(FileMoverDaemon& other) = delete;
    void operator=(const FileMoverDaemon& other) = delete;

    ~FileMoverDaemon();

    static FileMoverDaemon* getInstance();

    void initialize(const std::string& configPath);

    void run();
    void stop();

private:
    FileMoverDaemon();

    void createPid();
    void moveFiles();

    void destructOldPid();

    bool pathExist(const std::string& path) const;
};