#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <regex>

// singleton daemon class
class DaemonReminder {
    // value which used if config contains incorrect sleep time
    const std::chrono::seconds DEFAULT_TIME = std::chrono::seconds(10);
    const std::string PID_PATH = "/var/run/daemon_reminder.pid"; // path to PID file
    
    std::string configPath; // path to config file
    bool isTerminated = false; // flag to check if need to terminate
    std::chrono::seconds sleepTime; // time interval between moments of activity
    
    // structure containing the necessary information about the event
    struct Event {
        std::chrono::system_clock::time_point time; // time to notification
        std::chrono::seconds repeatInterval; // time between notification repetitions
        std::string message; // text of notification
      
        Event(std::chrono::system_clock::time_point t, std::chrono::seconds ri, std::string text):
            time(t), repeatInterval(ri), message(text) {}
      
        Event(const Event&) = default;
        Event& operator=(const Event&) = default;
    };
    
    std::vector<Event> events; // set of monitored events
    
    
    // Transform process into daemon
    void MakeDaemon(void);
    
    // Parse event string
    void ParseEvent(std::string& line, std::regex& eventFmt);
    
    // Restart Protection
    void CheckPid(void);
    void WritePid(void);
    
    // Constructors must conform to the singleton pattern
    DaemonReminder() {}
    DaemonReminder(const DaemonReminder&) = delete;
    DaemonReminder& operator=(const DaemonReminder&) = delete;
    
public:
    // Initialization function
    void Init(std::string configPath);
    
    // Load config file
    void LoadConfig(void);
    
    // Terminate daemon work
    void Terminate(void);
    
    // Get instance function
    static DaemonReminder& GetInstance(void) {
        static DaemonReminder instance;
        return instance;
    }
    
    // Function to start work
    void Run(void);
};