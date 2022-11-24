#pragma once

#include <string>
#include <chrono>

class DaemonFileRemover
{
private:
    const std::string pid_path = "/var/run/daemon_file_cleaner.pid";
    const std::string trigger_File = "dont.erase";
    const std::chrono::seconds def_sleep_time = std::chrono::seconds(15);

    std::string config_abs_path;
    std::string target_dir;
    std::chrono::seconds sleep_time;
    bool is_Terminate = false;

    DaemonFileRemover(){};
    DaemonFileRemover(const DaemonFileRemover &) = delete;
    DaemonFileRemover &operator=(const DaemonFileRemover &) = delete;

    void CreateDaemon();
    void KillDaemon();

public:
    static DaemonFileRemover &GetInstance()
    {
        static DaemonFileRemover instance;
        return instance;
    }

    void Init(const std::string& config_local);

    // SIGTERM handler
    void Terminate();

    // SIGHUP handler
    void ReadConfig();

    void Run();
};