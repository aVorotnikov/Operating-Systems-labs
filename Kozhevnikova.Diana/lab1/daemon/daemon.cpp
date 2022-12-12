#include "daemon.h"

#include <chrono>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <fstream>
#include <filesystem>
#include <csignal>
#include <unistd.h>

Daemon Daemon::instance;

void signalHandler(int signal) {
    auto& instance = Daemon::instance;
    switch(signal) {
        case SIGHUP:
            read(instance.configPath, instance.data);
            break;
        case SIGTERM:
            instance.terminate();
            break;
        default:
            break;
    }
}

namespace {
    void checkPid(const std::string& pidPath) {
        syslog(LOG_INFO, "Checing pid file");
        std::ifstream pidFile(pidPath);
        if (pidFile.is_open()) {
            pid_t pid = 0;
            if (pidFile >> pid && kill(pid, 0) == 0) {
                syslog(LOG_INFO, "Killing another daemon instance");
                kill(pid, SIGTERM);
            }
        pidFile.close();
        }
    }

    void forkDaemon() {
        syslog(LOG_INFO, "Forking process");
        int stdinCopy = dup(STDIN_FILENO);
        int stdoutCopy = dup(STDOUT_FILENO);
        int stderrCopy = dup(STDERR_FILENO);

        pid_t pid = fork();
        if (pid < 0) {
            syslog(LOG_ERR, "Forking error");
            exit(EXIT_FAILURE);
        } 
        else if (pid > 0) {
            syslog(LOG_INFO, "Kill parent process");
            exit(EXIT_SUCCESS);
        }
        umask(0);
        if (setsid() < 0) {
            syslog(LOG_ERR, "Forking error");
            exit(EXIT_FAILURE);
        }
        if (chdir("/") < 0) {
            syslog(LOG_ERR, "Forking error");
            exit(EXIT_FAILURE);
        }
        for (long x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
            close(x);
        dup2(stdinCopy, STDERR_FILENO);
        dup2(stdoutCopy, STDOUT_FILENO);
        dup2(stderrCopy, STDERR_FILENO);
    }

    void writePid(const std::string& pidPath) {
        syslog(LOG_INFO, "Writing to PID file");
        std::ofstream pidFile(pidPath);
        if (!pidFile.is_open()) {
            syslog(LOG_ERR, "Error while opening PID file");
            exit(EXIT_FAILURE);
        }
        pidFile << getpid();
        pidFile.close();
    }

    void setSignals() {
        std::signal(SIGHUP, signalHandler);
        std::signal(SIGTERM, signalHandler);
    }
}

void Daemon::copy() {
    static constexpr char subfolder_new[] = "NEW";
    static constexpr char subfolder_old[] = "OLD";
    unsigned diff_copy_time = 180;
    if (!std::filesystem::is_directory(data.src)) {
        syslog(LOG_WARNING, "Source directory does not exist");
        return;
    }
    if (!std::filesystem::is_directory(data.dst)) {
        syslog(LOG_WARNING, "Destination directory does not exist");
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(data.dst))
        std::filesystem::remove_all(entry.path());

    std::filesystem::create_directory(std::filesystem::path(data.dst) / subfolder_new);
    std::filesystem::create_directory(std::filesystem::path(data.dst) / subfolder_old);
    struct stat statbuf;

    for(const auto& entry: std::filesystem::directory_iterator(data.src)) {
        struct tm *file_time_tm;
        stat(entry.path().c_str(), &statbuf);
        file_time_tm = localtime(&statbuf.st_mtime);
        auto file_time = timelocal(file_time_tm);

        auto time_now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        
        std::filesystem::path dest_path = std::filesystem::path(data.dst) / 
        ((time_now - file_time) >= diff_copy_time ? subfolder_old : subfolder_new ) / entry.path().filename();

        std::filesystem::copy_file(entry.path(), dest_path, std::filesystem::copy_options::skip_existing);
    }
}

void Daemon::initialize(const std::string& path) {
    openlog("copier_daemon", LOG_NDELAY | LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Initializing daemon");

    configPath = std::filesystem::absolute(path);
    config::read(configPath,data);
    checkPid(pidPath);
    forkDaemon();
    writePid(pidPath);
    setSignals();
    isWorking = true;
    syslog(LOG_INFO, "Daemon initialized");
}

void Daemon::terminate() {
    isWorking = false;
    syslog(LOG_INFO, "Daemon terminated");
    closelog();

}

void Daemon::run(){
    while(isWorking){
        syslog(LOG_INFO, "Daemon start");
        copy();
        sleep(data.duration);
    }
}