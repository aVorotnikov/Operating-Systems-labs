#include "daemon.h"
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <thread>
#include <csignal>
#include <cmath>

void DaemonReminder::Terminate(void) {
    isTerminated = true;
    closelog();
}

static void _signalHandler(int sig) {
    switch (sig) {
    case SIGHUP:
        syslog(LOG_INFO, "Config reloading");
        DaemonReminder::GetInstance().LoadConfig();
        break;
    case SIGTERM:
        syslog(LOG_INFO, "Process terminated");
        DaemonReminder::GetInstance().Terminate();
        break;
    default:
        break;
    }
}

void DaemonReminder::MakeDaemon(void) {
    syslog(LOG_INFO, "Process daemonization starts");
    
    pid_t pid;
    auto DEV_NULL = open("/dev/null", O_RDWR);

    pid = fork();

    if (pid < 0) {
        syslog(LOG_ERR, "Forking error");
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    if (setsid() < 0) {
        syslog(LOG_ERR, "Group setting error");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") < 0) {
        syslog(LOG_ERR, "Directory changing error");
        exit(EXIT_FAILURE);
    }

    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
        close(x);

    dup2(DEV_NULL, STDIN_FILENO);
    dup2(DEV_NULL, STDOUT_FILENO);
    dup2(DEV_NULL, STDERR_FILENO);

    syslog(LOG_INFO, "Process daemonization ends");
}

void DaemonReminder::ParseEvent(std::string& line, std::regex& eventFmt) {
    std::vector<std::smatch> matches(std::sregex_iterator(line.begin(), line.end(), eventFmt),
        std::sregex_iterator());
    std::istringstream ss(matches[0][1]);
    std::tm eventTime_tm;
    ss >> std::get_time(&eventTime_tm, "%d/%m/%Y %T");
    auto eventTime = std::chrono::system_clock::from_time_t(std::mktime(&eventTime_tm));
    auto now = std::chrono::system_clock::now();
    if (ss.fail()) {
        syslog(LOG_WARNING, "Failed to parse event string: %s. It will be ignored", line.c_str());
    }
    else {
        std::chrono::seconds period(0);
        if (matches[0][2] != "") {
            if (matches[0][2] == "-h")
                period = std::chrono::seconds(3600);
            else if (matches[0][2] == "-d")
                period = std::chrono::seconds(24 * 3600);
            else
                period = std::chrono::seconds(7 * 24 * 3600);
        }
        
        if (now > eventTime) {
            if (period != std::chrono::seconds(0)) {
                eventTime += std::chrono::duration_cast<std::chrono::seconds>(
                  (((now - eventTime) / period) + ((now - eventTime) % period == std::chrono::seconds(0) ? 0 : 1)) * period);
            }
            else {
                syslog(LOG_WARNING, "Event time has passed: %s. It will be ignored", line.c_str());
                return;
            }
        }

        events.push_back(Event(eventTime, period, matches[0][3]));
    }
}

void DaemonReminder::LoadConfig(void) {
    syslog(LOG_INFO, "Load config");
    sleepTime = DEFAULT_TIME;
    events.clear();

    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        syslog(LOG_ERR, "Invalid config");
        isTerminated = true;
        return;
    }

    std::regex commentFmt(R"(^\s*#.*\s*)"),
        timeFmt(R"(^\s*interval:\s*(\d+)\s*$)"),
        eventFmt(R"(^\s*add_event\s+(\S+\s+\S+)\s+(-h|-d|-w)*\s*(.+)\s*)"),
        emptyFmt(R"(^\s*$)");
    std::string line;
    bool isSleepTimeSet = false;
    while (std::getline(configFile, line)) {
        if (std::regex_match(line, timeFmt) && !isSleepTimeSet) {
          // The time interval is set once on the first occurrence
          sleepTime = std::chrono::seconds(std::stoi((*std::sregex_iterator(line.begin(),
              line.end(), timeFmt))[1]));
          isSleepTimeSet = true;
        }
        else if (std::regex_match(line, eventFmt)) {
            // Parsing the adding event
            ParseEvent(line, eventFmt);
        }
        else if (!std::regex_match(line, commentFmt) && !std::regex_match(line, emptyFmt)) {
            syslog(LOG_WARNING, "Failed to parse string: %s. It will be ignored", line.c_str());
        }
        else {
            // Ignore comments and empty lines
            ;
        }
    }

    if (!isSleepTimeSet)
        syslog(LOG_WARNING, "The time interval is not set. The default value is used");
    if (events.empty())
        syslog(LOG_WARNING, "No events to remind");
    else
        syslog(LOG_INFO, "Have %li events to remind", events.size());

    syslog(LOG_INFO, "Config loaded successfully");
}

void DaemonReminder::CheckPid(void) {
    syslog(LOG_INFO, "Checking if daemon already running");
    std::ifstream pidFile(PID_PATH);
    if (pidFile.is_open()) {
        int pid = 0;
        if (pidFile >> pid && !kill(pid, 0)) {
            syslog(LOG_WARNING, "Stopping a previously running daemon. PID: %i", pid);
            kill(pid, SIGTERM);
        }
    }
}

void DaemonReminder::WritePid(void) {
    syslog(LOG_INFO, "Writing PID");
    std::ofstream pidFile(PID_PATH.c_str());
    if (!pidFile.is_open()) {
        syslog(LOG_ERR, "Writing pid failed");
        exit(EXIT_FAILURE);
    }
    pidFile << getpid();
    syslog(LOG_INFO, "Writing pid successed");
}

void DaemonReminder::Init(std::string configFilePath) {
    char buf[PATH_MAX];
    getcwd(buf, sizeof(buf));
    configPath = buf;
    configPath += "/" + configFilePath;

    openlog("DaemonReminder", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Daemon initialization");

    CheckPid();
    MakeDaemon();
    WritePid();

    syslog(LOG_INFO, "Set signal handlers");
    std::signal(SIGHUP, _signalHandler);
    std::signal(SIGTERM, _signalHandler);

    LoadConfig();
    syslog(LOG_INFO, "Daemon initialization successed");
}

void DaemonReminder::Run(void) {
    while (!isTerminated) {
        auto now = std::chrono::system_clock::now();
        auto nextEventTime = now + sleepTime;
        if (!events.empty()) {
            for (auto i = events.begin(); i != events.end(); ++i) {
                if (i->time <= now) {
                    std::string text = "gnome-terminal -- bash -c \"echo '" + i->message + "'; read n\"";
                    system(text.c_str());
                    if (i->repeatInterval != std::chrono::seconds(0))
                        i->time += i->repeatInterval;
                    else
                        events.erase(i--);
                }
                if (i->time < nextEventTime)
                    nextEventTime = i->time;
            }
        }

        std::this_thread::sleep_until(now + sleepTime < nextEventTime ? now + sleepTime : nextEventTime);
    }
}