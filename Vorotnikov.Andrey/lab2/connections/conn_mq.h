#pragma once

#include "conn.h"

#include <mqueue.h>

#include <string>

class ConnectionMq : public Connection
{
public:
    ConnectionMq(pid_t pid, Connection::Type type);
    ~ConnectionMq();
    bool Read(void* buf, const std::size_t count) override;
    bool Write(void* buf, const std::size_t count) override;

private:
    static constexpr char filePathTemplate_[] = "/tmp/mq_";
    static constexpr std::size_t msgSize = 64;
    const std::string absPath_;
    mqd_t mqd_;
};
