#pragma once

#include <string>
#include "sys/types.h"
#include "utils/message.h"

class Connection
{
public:
    virtual ~Connection() = default;

    static Connection *createDefault(const std::string &name, bool isHost);

    virtual bool open() = 0;
    virtual bool read(Message &msg) = 0;
    virtual bool write(const Message &msg) = 0;
    virtual bool close() = 0;
};
