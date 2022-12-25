#pragma once

#include "conn.h"

class ConnectionSeg : public Connection
{
public:
    ConnectionSeg(pid_t pid, Connection::Type type);
    ~ConnectionSeg();
    bool Read(void* buf, const std::size_t count) override;
    bool Write(void* buf, const std::size_t count) override;

private:
    static constexpr std::size_t segSize_ = sizeof(int);
    int shmId_;
    void* segPtr_;
};
