#pragma once

#include "conn.h"

#include <string>

class Fifo : public Connection
{
private:
    const std::string FIFO_ROUTE = "/tmp/fifo_conn_";

    bool isHost = false;
    std::string name;
    int descriptor;

public:
    Fifo() = default;
    ~Fifo() = default;

    bool open(const pid_t &pid, const bool &isHost) final;
    bool read(Message &msg) const final;
    bool write(const Message &msg) final;
    bool close() final;
};