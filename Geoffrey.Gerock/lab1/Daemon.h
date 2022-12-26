#pragma once

#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <syslog.h>
#include <sys/stat.h>
#include "ConfigReader.h"

class Daemon {
public:
    static Daemon& getInstance() {
        static Daemon instance;
        return instance;
    }

    Daemon() = default;

    Daemon(const Daemon&) = delete;

    Daemon& operator=(const Daemon&) = delete;

    static void init() noexcept;

    void setConfigPath(const fs::path& configPath);

    static void readConfig() noexcept;

    void run() noexcept;

private:
    static inline const std::string _daemonName = "deleter";
    static inline const fs::path _pidPath = fs::path("/var/run/" + _daemonName + ".pid");
    static inline const int _sleepTime = 20;
    const fs::path _configPath;
    ConfigReader _config;
    static inline bool _needTerminate = false;

    static void _setUpLogging();

    static void _tearDownLogging();

    static void _checkPid();

    static void _fork();

    static void _writePid();

    static void _sigHandler(int sig);

    static void _fdRedirect();

    void _doJob();

    static void _terminate(const int status);
};
