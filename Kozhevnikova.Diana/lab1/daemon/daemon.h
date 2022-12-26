#pragma once

#include "../config/config.h"

class Daemon {
private:    
    constexpr static char pidPath[] = "/var/run/daemon.pid";
    bool isWorking = false;
    config::Data data;
    std::string configPath;
    void copy();
    static Daemon instance;
    Daemon() = default;
    Daemon(const Daemon&) = delete;
    Daemon& operator=(Daemon&) = delete;
public:
    static Daemon& getInstance() {
        return instance;
    }
    friend void signalHandler(int signal);
    void initialize(const std::string& path);
    void terminate();
    void run();
};