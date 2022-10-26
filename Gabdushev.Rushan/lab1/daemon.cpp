#include "daemon.hpp"
#include <limits.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <csignal>
#include <fstream>
#include <fcntl.h>
#include <filesystem>
#include <thread>

static void _handleSignal(int signal)
{
    switch (signal)
    {
    case SIGTERM:
        syslog(LOG_INFO, "Process terminated");
        DaemonTmpCleaner::Get().Terminate();
        break;
    case SIGHUP:
        syslog(LOG_INFO, "Config reloading");
        DaemonTmpCleaner::Get().ReloadConfig();
        break;
    default:
        break;
    }
}

void _deleteFilesWithExtension(const std::string& directory, const std::string& extension)
{
    for (auto &file : std::filesystem::directory_iterator(directory))
    {
        if (file.path().extension() == extension)
        {
            if (!std::filesystem::remove(file))
            {
                syslog(LOG_WARNING, "Failed to remove file: \"%s\"", file.path().c_str());
            }
        }
    }
}

void DaemonTmpCleaner::KillExistingDaemon()
{
    syslog(LOG_INFO, "Checking for existing daemon process");
    std::ifstream pidFile(pidPath);
    if (pidFile.is_open())
    {
        int runningDaemonPid = 0;
        if (pidFile >> runningDaemonPid && kill(runningDaemonPid, 0) == 0)
        {
            syslog(LOG_WARNING, "Stopping a currently existing daemon with PID: %i", runningDaemonPid);
            kill(runningDaemonPid, SIGTERM);
        }
    }
}

void DaemonTmpCleaner::WritePid()
{
    syslog(LOG_INFO, "Writing pid");
    std::ofstream pidFile(pidPath.c_str());
    if (!pidFile.is_open())
    {
        syslog(LOG_ERR, "Writing pid error");
        exit(EXIT_FAILURE);
    }
    pidFile << getpid();
    syslog(LOG_INFO, "Writing pid successed");
}

void DaemonTmpCleaner::Daemonize()
{
    syslog(LOG_INFO, "Daemonization starts");

    pid_t pid = fork();
    if (pid < 0)
    {
        syslog(LOG_ERR, "Fork error");
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    umask(0);
    if (setsid() < 0)
    {
        syslog(LOG_ERR, "Group setting error");
        exit(EXIT_FAILURE);
    }
    if (chdir("/") < 0)
    {
        syslog(LOG_ERR, "Switching to root dir error");
        exit(EXIT_FAILURE);
    }
    // Close all file descriptors
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
    {
        close(x);
    }

    int devNull = open("/dev/null", O_RDWR);
    dup2(devNull, STDIN_FILENO);
    dup2(devNull, STDOUT_FILENO);
    dup2(devNull, STDERR_FILENO);
    syslog(LOG_INFO, "Daemonization ends");
}

void DaemonTmpCleaner::Initialize(const std::string& configLocalPath)
{
    configAbsPath = std::filesystem::absolute(configLocalPath);

    openlog("DaemonTmpCleaner", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Daemon initialization");

    KillExistingDaemon();
    Daemonize();

    syslog(LOG_INFO, "Setting signal handlers");
    std::signal(SIGHUP, _handleSignal);
    std::signal(SIGTERM, _handleSignal);

    WritePid();

    ReloadConfig();
    syslog(LOG_INFO, "Daemon initialization finished");
}

void DaemonTmpCleaner::Run()
{
    while (!isTerminateReceived)
    {
        if (!std::filesystem::is_directory(targetDirPath))
        {
            syslog(LOG_WARNING, "Target directory path from config file does not exist, keep waiting");
        }
        else
        {
            _deleteFilesWithExtension(targetDirPath, targetFileExtention);
        }
        std::this_thread::sleep_for(sleepTime);
    }
}

void DaemonTmpCleaner::Terminate()
{
    isTerminateReceived = true;
    closelog();
}

void DaemonTmpCleaner::ReloadConfig()
{
    std::ifstream configFile(configAbsPath);
    if (!configFile.is_open())
    {
        syslog(LOG_ERR, "Invalid config file");
        exit(EXIT_FAILURE);
    }
    if (!std::getline(configFile, targetDirPath))
    {
        syslog(LOG_ERR, "Can not read target directory path from config file");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_INFO, "Set target directory path to \"%s\"", targetDirPath.c_str());
    int sleepTimeSeconds = -1;
    if (!(configFile >> sleepTimeSeconds))
    {
        syslog(LOG_WARNING, "Can not read sleep time from config file, using default value");
        sleepTime = defaultSleepTime;
    }
    else if (sleepTimeSeconds <= 0)
    {
        syslog(LOG_WARNING, "Sleep time from config file is not positive, using default value");
        sleepTime = defaultSleepTime;
    }
    else
    {
        sleepTime = std::chrono::seconds(sleepTimeSeconds);
    }
    syslog(LOG_INFO, "Sleep time set to %li seconds", sleepTime.count());
}