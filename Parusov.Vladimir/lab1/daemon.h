#pragma once
#include <string>

// singleton Daemon representation class
class Daemon
{
private:
    // path to PID file
    const std::string PID_FILE_PATH = "/var/run/lab1.pid";
    // default value, which used if config became incorrect during daemon work
    const unsigned int DEFAULT_SLEEP_TIME = 10;
    // path to config file
    std::string m_configPath;
    // flag to check if we still working
    bool m_isRunning = true;
    // flag to check if our config is valid
    bool m_isValidConfig = false;
    // time we need to restart our process. In seconds
    unsigned int m_sleepTime = DEFAULT_SLEEP_TIME;
    // path to directory we need to clean
    std::string m_cleanPath;

    // variable for singleton pattern
    static Daemon m_daemonInstance;

    // Function to transform our process into Deamon
    void StartDaemon(void);

    // Function to parse config file
    void ParseConfig(void);

    // Private constructor so no one can initialize this class except singleton
    Daemon() {}
    // Theese constructors are need to be blocked as well
    Daemon(const Daemon& root) = delete;
    Daemon& operator=(const Daemon&) = delete;
public:

    // Initialization function
    void Init(std::string configFilePath);

    // Get instance function
    static Daemon &GetInstance(void) { return m_daemonInstance; }

    // Function to ask Daemon to reload config
    void ReloadConfig(void);

    // Function to ask Daemon to terminate its work
    void Terminate(void);

    // Function to ask Daemon to start its work
    void Run(void);

    // Function which contains actual code for solving task
    void DoWork(void);
};