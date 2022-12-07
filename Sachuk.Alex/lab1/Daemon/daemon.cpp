#include <syslog.h>
#include <csignal>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <string>

#include "../ConfigReader/configReader.h"
#include "daemon.h"


bool Daemon::isPidRegistered(int& foundedPid) {
    std::ifstream pidFile(PID_PATH);
    if (pidFile.is_open()) {
        if (pidFile >> foundedPid && kill(foundedPid, 0) == 0) {
            pidFile.close();
            return true;
        }
        pidFile.close();
    }
    return false;
}


void Daemon::MainInit() {
    int stdinCopy = dup(STDIN_FILENO);
    int stdoutCopy = dup(STDOUT_FILENO);
    int stderrCopy = dup(STDERR_FILENO);
    pid_t pid = fork();

    if (pid < 0) {
        syslog(LOG_ERR, "Fork error.");
        syslog(LOG_INFO, "Stop initializing...");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
        exit(EXIT_SUCCESS);
    else {
        umask(0);
        
        if (setsid() < 0) {
            syslog(LOG_ERR, "Group setting error.");
            syslog(LOG_INFO, "Stop initializing...");
            exit(EXIT_FAILURE);
        }
        
        if (chdir("/") < 0) {
            syslog(LOG_ERR, "Changing directory error.");
            syslog(LOG_INFO, "Stop initializing...");
            exit(EXIT_FAILURE);
        }

        for (int x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
            close(x); 

        dup2(stdinCopy, STDIN_FILENO);
        dup2(stdoutCopy, STDOUT_FILENO);
        dup2(stderrCopy, STDERR_FILENO);
    }
}


void Daemon::RegisterPid() {
    std::ofstream pidFile(PID_PATH, std::ios_base::out);
    
    if (!pidFile.is_open()) {
        syslog(LOG_ERR, "Can't open pid file!");
        exit(EXIT_FAILURE);
    }
    
    pidFile << getpid();
    pidFile.close();
}


void Daemon::ReinitLogger() {
    syslog(LOG_INFO, "Config reloading...");
    ConfigValues values;
    if (ConfigReader::ReadConfig(globalConfigPath, LoggerConfigDescription(), values)) {
        logger.Init(values);
        currentRefreshTime = logger.GetRefreshDuration();
        syslog(LOG_INFO, "Config reloaded succesfully.");
    }
    else
        syslog(LOG_ERR, "Cant reload config params: bad path or values in config!");
}

void Daemon::Terminate() {
    syslog(LOG_INFO, "Terminating process...");
    wasTerminated = true;
    syslog(LOG_INFO, "Process terminated.");
    closelog();
}

static void SignalManager(int signal) {
    switch (signal) {
    case SIGHUP:
        Daemon::GetDaemonInstance().ReinitLogger();
        break;
    case SIGTERM:
        Daemon::GetDaemonInstance().Terminate();
        break;
    default:
        break;
    }
}

void Daemon::SetSignalsManagment() {
    std::signal(SIGHUP, SignalManager);
    std::signal(SIGTERM, SignalManager);
}

// Default initializer method
void Daemon::Init(const std::string& localConfigPath) {
    openlog("daemonlog", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Starting daemon initialization...");
    
    // load and save global config path
    char workingDirPath[300];
    getcwd(workingDirPath, sizeof(workingDirPath));
    globalConfigPath = std::string(workingDirPath) + "/" + localConfigPath;

    // Check if daemon pid registered
    syslog(LOG_INFO, "Checking is daemon already started...");
    int pid = 0;
    if (isPidRegistered(pid)) {
        syslog(LOG_INFO, "The daemon already started! Killing old process...");
        kill(pid, SIGTERM);
    }
    else
        syslog(LOG_INFO, "Daemon want't started.");

    // Initzialize daemon
    syslog(LOG_INFO, "Initialize daemon...");
    MainInit();

    // Register daemon
    syslog(LOG_INFO, "Register daemon...");
    RegisterPid();

    // Set signals managers
    syslog(LOG_INFO, "Set signals callback function...");
    SetSignalsManagment();

    // Make first config initialization
    ReinitLogger();

    // Finish init
    syslog(LOG_INFO, "Initialazion completed succesfully!");
}

void Daemon::Run() {
    while (!wasTerminated) {
        //std::string toLog = "Going to log files from '" + logger.GetFirstDir() + "' to '" + logger.GetSecondDir() + "'";
        //const char* str = toLog.c_str();
        syslog(LOG_INFO, "Going to log files...");
        if (logger.Log() == false)
            syslog(LOG_INFO, "Can't log files: some of directories' paths are bad.");
        std::this_thread::sleep_for(std::chrono::seconds(currentRefreshTime));
    }
}
