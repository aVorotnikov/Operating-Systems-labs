#pragma once

#include "sys/types.h"
#include "utils/message.h"

class Connection
{
public:
    virtual ~Connection() = default;

    static Connection *create();

    virtual bool open(const pid_t &pid, const bool &isHost) = 0;
    virtual bool read(Message &msg) const = 0;
    virtual bool write(const Message &msg) = 0;
    virtual bool close() = 0;
};