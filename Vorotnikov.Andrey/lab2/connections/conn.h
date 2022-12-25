#pragma once

#include <sys/types.h>
#include <memory>

class Connection
{
public:
    enum class Type
    {
        Host,
        Client
    };

    static std::shared_ptr<Connection> GetConnection(pid_t pid, Type type);

    Connection(pid_t pid, Type type);
    virtual ~Connection() = default;

    virtual bool Read(void* buf, const std::size_t count) = 0;
    virtual bool Write(const void* buf, const std::size_t count) = 0;

    const pid_t hostPid_;
    const Type type_;
};
