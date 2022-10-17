#pragma once

#include "../Logger/logger.h"

// Daemon class-Singleton
class Daemon {
public:
    // Delete copy and move constructors
    Daemon(const Daemon&) = delete;
    Daemon(Daemon&&) = delete;

    // Default initializer method
    void Init(const std::string& localConfigPath);

    // Run daemon method
    void Run();

    // Reload config method
    void ReinitLogger();

    // Termintate method
    void Terminate();

    // Get Daemon instance method
    static Daemon& GetDaemonInstance() { 
        // Daemon instance
        static Daemon daemonInstance;
        return daemonInstance; 
    };
private:
    // private constructor
    Daemon() {};

    // Methods to 'Init' daemon in system
    bool isPidRegistered(int& foundedPid);
    void MainInit();
    void RegisterPid();
    void SetSignalsManagment();

    // Variables and constants
    const std::string PID_PATH = "/var/run/lab1_10.pid"; // pid file path
    const uint STANDART_REFRESH_TIME_IN_SEC = 20;        // Refresh duration

    uint currentRefreshTime = 
        STANDART_REFRESH_TIME_IN_SEC;                    // Current refresh duration
    std::string globalConfigPath;                        // config file path
    Logger logger;                                       // 'Logger' class instance
    bool wasTerminated = false;                          // Flag: is 'true' when comes SIGTERM
};
