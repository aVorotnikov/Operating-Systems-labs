#pragma once

#include "../Logger/logger.h"

// Daemon class-Singleton
class Daemon {
public:
    // Delete copy and l-reference constructors
    Daemon(const Daemon&) = delete;
    Daemon(Daemon&&) = delete;

    // Default initializer method
    bool Init(const std::string& localConfigPath);

    // Run daemon method

private:
    // private constructor
    Daemon() {};

    // Daemon instance
    static Daemon daemonInstance;

    // Variables and constants
    const std::string PID_PATH = "/var/run/lab1_10.pid";    // pid file path
    std::string globalConfigPath;                           // config file path
    Logger logger;                                          // 'Logger' class instance
};
