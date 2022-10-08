/// @file
/// @brief Определение класса демона
/// @author Воротников Андрей

#include "daemon.h"

#include <unistd.h>
#include <csignal>
#include <syslog.h>
#include <sys/stat.h>
#include <fstream>

extern Daemon Daemon::instance;

void SignalHandler(const int sig)
{
    auto& instance = Daemon::instance;
    switch(sig)
    {
    case SIGHUP:
        instance.onReloadConfig_(instance.configPath_);
        instance.updated_ = true;
        instance.work_.notify_one();
        break;
    case SIGTERM:
        instance.onTerminate_();
        instance.needWork_ = false;
        instance.work_.notify_one();
        break;
    default:
        break;
    }
}

namespace
{

void CheckPid(const std::string& pidFileName)
{
    syslog(LOG_INFO, "Checking if daemon already running");
    std::ifstream pidFile(pidFileName);
    if (pidFile.is_open())
    {
        int pid = 0;
        if (pidFile >> pid && !kill(pid, 0))
        {
            syslog(LOG_WARNING, "Killing daemon. PID: %i", pid);
            kill(pid, SIGTERM);
        }
    }
}

void Forking()
{
    syslog(LOG_INFO, "Forking");
    auto stdin_copy = dup(STDIN_FILENO);
    auto stdout_copy = dup(STDOUT_FILENO);
    auto stderr_copy = dup(STDERR_FILENO);

    auto pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
        syslog(LOG_ERR, "Forking error");
    }
    if (0 != pid)
        exit(EXIT_SUCCESS);
    umask(0);
    if (chdir("/") < 0)
    {
        syslog(LOG_ERR, "Failed to change directory");
        exit(EXIT_FAILURE);
    }
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
        close(x);

    dup2(stdin_copy, STDIN_FILENO);
    dup2(stdout_copy, STDOUT_FILENO);
    dup2(stderr_copy, STDERR_FILENO);

    syslog(LOG_INFO, "Forking successed");
}

void WritePid(const std::string& pidFileName)
{
    syslog(LOG_INFO, "Writing PID");
    std::ofstream pidFile(pidFileName);
    if (!pidFile.is_open())
    {
        syslog(LOG_ERR, "Writing pid failed - cannot open '%s' file", pidFileName.c_str());
        exit(EXIT_FAILURE);
    }
    pidFile << getpid();
    syslog(LOG_INFO, "Writing pid successed");
}

void SetSignals()
{
    signal(SIGHUP, SignalHandler);
    signal(SIGTERM, SignalHandler);
}

} // anonymous namespace

Daemon& Daemon::GetRef()
{
    return instance;
}

Daemon::Daemon() : updated_(false), needWork_(false)
{
}

bool Daemon::SetParams(
    const std::string& path,
    const std::function<void ()>& onWork,
    const std::function<void (const std::string&)>& onReloadConfig,
    const std::function<void ()>& onTerminate,
    const std::chrono::seconds& duration
)
{
    if (needWork_)
        return false;
    needWork_ = true;
    configPath_ = std::filesystem::absolute(path);
    onWork_ = onWork;
    onReloadConfig_ = onReloadConfig;
    onTerminate_ = onTerminate;
    duration_ = duration;
    return true;
}

void Daemon::UpdateDuration(const std::chrono::seconds& duration)
{
    duration_= duration;
}

void Daemon::Init()
{
    openlog("copying_daemon", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Start daemon initialization");
    CheckPid(pidFilePath);
    Forking();
    WritePid(pidFilePath);
    SetSignals();
    onReloadConfig_(configPath_);
}

bool Daemon::Run()
{
    if (!needWork_)
        return false;
    while (needWork_)
    {
        updated_ = false;
        onWork_();
        std::unique_lock lock(workMtx_);
        work_.wait_for(lock, duration_, [daemon = this]() {return !daemon->needWork_ || daemon->updated_;});
    }
    syslog(LOG_INFO, "Daemon finished");
    closelog();
    return true;
}
