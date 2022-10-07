/// @file
/// @brief Определение класса демона
/// @author Воротников Андрей

#include "daemon.h"

#include <signal.h>

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
    configPath_ = path;
    onWork_ = onWork;
    onReloadConfig_ = onReloadConfig;
    onTerminate_ = onTerminate;
    duration_ = duration;
    return true;
}

bool Daemon::Run()
{
    if (!needWork_)
        return false;
    while (true)
    {
        std::unique_lock lock(workMtx_);
        work_.wait_for(lock, duration_, [daemon = this]() {return daemon->needWork_;});
        if (!needWork_)
            break;
        onWork_();
    }
    return true;
}
