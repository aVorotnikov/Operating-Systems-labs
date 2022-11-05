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

#include "daemon.hpp"

void Daemon::terminate(void) {
    Daemon::isTerminated = true;
    closelog();
}

static void _signalHandler(int sig) {
    switch (sig) {
    case SIGHUP:
        syslog(LOG_INFO, "Config reloading");
        Daemon::getInstance().loadConfig();
        break;
    case SIGTERM:
        syslog(LOG_INFO, "Process terminated");
        Daemon::getInstance().terminate();
        break;
    default:
        break;
    }
}

void Daemon::checkPid(void) {
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

void Daemon::writePid(void) {
    syslog(LOG_INFO, "Writing PID");
    std::ofstream pidFile(Daemon::PID_PATH);
    if (!pidFile.is_open()) {
        syslog(LOG_ERR, "Writing pid failed");
        exit(EXIT_FAILURE);
    }
    pidFile << getpid();
    syslog(LOG_INFO, "Writing pid successed");
}

void Daemon::makeDaemon(void) {
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

void Daemon::loadConfig(void) {
    syslog(LOG_INFO, "Load config");
    Daemon::data.clear();

    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        syslog(LOG_ERR, "Invalid config");
        isTerminated = true;
        return;
    }

    std::regex commentFmt(R"(^\s*#.*\s*)"),
        timeFmt(R"(^\s*interval:\s*(\d+)\s*$)"),
        dataFmt(R"(^\s*(?:(\S+)|\"([\S\s]+)\")\s+(?:(\S+)|\"([\S\s]+)\")\s*$)"),
        emptyFmt(R"(^\s*$)");
    std::string line;
    while (std::getline(configFile, line)) {
        if (std::regex_match(line, timeFmt)){
            sleepTime = std::chrono::seconds(std::stoi((*std::sregex_iterator(line.begin(), line.end(), timeFmt))[1]));
        }
        else if (std::regex_match(line, dataFmt)) {
            std::vector<std::smatch> matches(std::sregex_iterator(line.begin(), line.end(), dataFmt), std::sregex_iterator());
            std::string directory = (matches[0][1].length() != 0) ? matches[0][1].str() : matches[0][2].str();
            std::string file = (matches[0][3].length() != 0) ? matches[0][3].str() : matches[0][4].str();
            syslog(LOG_INFO, "Directory: %s; File: %s;", directory.c_str(), file.c_str());
            Daemon::data.push_back({directory, file});
        }
        else if (std::regex_match(line, commentFmt) || std::regex_match(line, emptyFmt)) {
            // Ignore comments and empty lines
        }
        else {
            syslog(LOG_WARNING, "Failed to parse string: %s. It will be ignored", line.c_str());
        }
    }
    syslog(LOG_INFO, "Config loaded successfully");
}

void Daemon::init(std::string configFilePath) {
    Daemon::directoryPath = std::filesystem::current_path();
    Daemon::configPath = directoryPath / std::filesystem::path{configFilePath};

    openlog("DaemonReminder", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Daemon initialization");

    Daemon::checkPid();
    Daemon::makeDaemon();
    Daemon::writePid();

    syslog(LOG_INFO, "Set signal handlers");
    std::signal(SIGHUP, _signalHandler);
    std::signal(SIGTERM, _signalHandler);

    Daemon::loadConfig();
    syslog(LOG_INFO, "Daemon initialization successed");
}

void Daemon::run(void) {
    while (!isTerminated) {
        auto now = std::chrono::system_clock::now();

        if (!data.empty()){
            for (auto dir_file: Daemon::data){
                auto dir = Daemon::directoryPath / std::filesystem::path(dir_file.directory);
                auto file = dir / dir_file.file;
                if (std::filesystem::exists(dir)){
                    if (!std::filesystem::exists(file)){    
                        for (auto const& dir_entry : std::filesystem::directory_iterator{dir})
                            std::filesystem::remove_all(dir_entry);
                        syslog(LOG_INFO, "Delete all in %s\n", dir.c_str());
                    }
                    else{
                        syslog(LOG_INFO, "Not delete in %s, find %s\n",  dir.c_str(), file.filename().c_str());
                    }
                }
                else{
                    syslog(LOG_WARNING, "%s does not exist\n", dir.c_str());
                }
            }
        }

        std::this_thread::sleep_until(now + sleepTime);
    }
}