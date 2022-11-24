/**
 * @file daemon.cpp
 * @author Baev Daniil (baev.daniil.2002@gmail.com)
 * @brief 
 * @version 0.2
 * @date 2022-11-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <fcntl.h>
#include <syslog.h>
#include <sys/stat.h>
#include <unistd.h>
#include <csignal>
#include <ctime>
#include <fstream>
#include <thread>

#include "daemon.hpp"

static void _sighandler(int sig) {
    switch (sig) {
    case SIGHUP:
        syslog(LOG_INFO, "Config reloading...");
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

void Daemon::terminate(void) {
    isTerminated = true;
    closelog();
}

void Daemon::checkPid(void) {
    syslog(LOG_INFO, "Checking pid");
    std::ifstream pidFile(PID_PATH);
    if (pidFile.is_open()) {
        int pid = 0;
        if (pidFile >> pid && !kill(pid, 0)) {
            kill(pid, SIGTERM);
        }
    }
}

void Daemon::writePid(void) {
    std::ofstream pidFile(PID_PATH);
    if (!pidFile.is_open()) {
        syslog(LOG_ERR, "Writing pid failed");
        exit(EXIT_FAILURE);
    }
    pidFile << getpid();
    syslog(LOG_INFO, "Writing pid successed");
}

void Daemon::daemonize(void) {
    syslog(LOG_INFO, "Daemonization...");
    
    pid_t pid;

    // fork off the parent process
    pid = fork();

    // an error occurred
    if (pid < 0) {
        syslog(LOG_ERR, "Forking error");
        exit(EXIT_FAILURE);
    }
    // success
    if (pid > 0)
        exit(EXIT_SUCCESS);

    // the child process becomes session leader
    if (setsid() < 0) {
        syslog(LOG_ERR, "Group setting error");
        exit(EXIT_FAILURE);
    }

    // set new file permissions    
    umask(0);

    // change working dir to root
    if (chdir("/") < 0) {
        syslog(LOG_ERR, "Directory changing error");
        exit(EXIT_FAILURE);
    }

    // close all open file descriptors
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
        close(x);

    // close stdin, stderr, stdout
    if (int fdnull = open("/dev/null", O_RDWR)){
        dup2(fdnull, STDIN_FILENO);
        dup2(fdnull, STDOUT_FILENO);
        dup2(fdnull, STDERR_FILENO);
        close(fdnull);
    }
    syslog(LOG_INFO, "Daemonization ends successfully");
}

void Daemon::loadConfig(void) {
    syslog(LOG_INFO, "Load config...");
    data.clear();

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
            data.push_back({directory, file});
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
    openlog("Daemon", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Daemon initialization...");

    directoryPath = std::filesystem::current_path();
    configPath = directoryPath / std::filesystem::path{configFilePath};
    loadConfig();

    checkPid();
    daemonize();
    writePid();

    // set signal handlers
    std::signal(SIGHUP, _sighandler);
    std::signal(SIGTERM, _sighandler);

    syslog(LOG_INFO, "Daemon initialization successed");
}

void Daemon::run(void) {
    while (!isTerminated) {
        auto now = std::chrono::system_clock::now();

        if (!data.empty())
            for (Data dir_file: data){
                std::filesystem::path dir = directoryPath / std::filesystem::path(dir_file.directory);
                std::filesystem::path file = dir / dir_file.file;
                if (std::filesystem::exists(dir))
                    if (!std::filesystem::exists(file)){    
                        for (auto const& dir_entry : std::filesystem::directory_iterator{dir})
                            std::filesystem::remove_all(dir_entry);
                        syslog(LOG_INFO, "Delete all in %s\n", dir.c_str());
                    }
                    else
                        syslog(LOG_INFO, "Not delete in %s, find %s\n",  dir.c_str(), file.filename().c_str());
                else
                    syslog(LOG_WARNING, "%s does not exist\n", dir.c_str());
            }
        std::this_thread::sleep_until(now + sleepTime);
    }
}