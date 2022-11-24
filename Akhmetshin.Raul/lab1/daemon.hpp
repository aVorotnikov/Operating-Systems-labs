#pragma once

#include <string>
#include <chrono>

class DaemonFileRemover
{
private:
    const std::string trigger_File = "dont.erase";
    const std::chrono::seconds def_sleep_time = std::chrono::seconds(15);
    const std::string pid_path = "/var/run/daemon_file_cleaner.pid";

    
    std::chrono::seconds sleep_time;
    bool is_Terminate = false;
    std::string config_abs_path;
    std::string target_dir;

    DaemonFileRemover(){};
    DaemonFileRemover(const DaemonFileRemover &) = delete;
    DaemonFileRemover &operator=(const DaemonFileRemover &) = delete;

    void CreateDaemon();
    void KillDaemon();

public:
    static DaemonFileRemover &GetInstance()
    {
        static DaemonFileRemover daemon;
        return daemon;
    }

    void Terminate();
    void ReadConfig();
    void Run();
    void Init(const std::string& config_local);
 
};