/// @file
/// @brief Определение класса демона
/// @author Воротников Андрей

#include "daemon.h"

#include <signal.h>
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
    std::ifstream pidFile(pidFileName);
    if (pidFile.is_open())
        if (int pid; pidFile >> pid && !kill(pid, 0))
            kill(pid, SIGTERM);
}

void Forking()
{
    auto stdin_copy = dup(STDIN_FILENO);
    auto stdout_copy = dup(STDOUT_FILENO);
    auto stderr_copy = dup(STDERR_FILENO);

    auto pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (0 != pid)
        exit(EXIT_SUCCESS);
    umask(0);
    if (chdir("/") < 0)
        exit(EXIT_FAILURE);
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
        close(x);

    dup2(stdin_copy, STDIN_FILENO);
    dup2(stdout_copy, STDOUT_FILENO);
    dup2(stderr_copy, STDERR_FILENO);
}

void WritePid(const std::string& pidFileName)
{
    std::ofstream pidFile(pidFileName);
    if (!pidFile.is_open())
        exit(EXIT_FAILURE);
    pidFile << getpid();
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

Daemon::Daemon() : needWork_(false)
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

void Daemon::Init()
{
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
        onWork_();
        std::unique_lock lock(workMtx_);
        work_.wait_for(lock, duration_, [daemon = this]() {return !daemon->needWork_;});
    }
    return true;
}
