/// @file
/// @brief Определение класса демона
/// @author Воротников Андрей

#include "daemon.h"

extern Daemon Daemon::instance;

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
    const std::function<bool ()>& onReloadConfig,
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
