#pragma once

#include <chrono>

class Config
{
public:
    static constexpr long SEMAPHORE_TIMEOUT_SECONDS = 5;
    static constexpr auto PING_MAX_INTERVAL = std::chrono::milliseconds(4000);
    static constexpr auto MAX_CLIENT_REPLY_DELAY_MILLISECONDS = 1500;
};