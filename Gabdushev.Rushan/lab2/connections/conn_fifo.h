#pragma once

#include "conn.h"

#include <string>

class Fifo : public Connection
{
private:
    static constexpr char PREFIX[] = "/tmp/wolf_n_goat_fifo_conn_";
    bool isHost = false;
    std::string name;
    int descriptor = -1;

public:
    Fifo() = delete;
    Fifo(const std::string &name, bool isHost);
    bool open();
    bool read(Message &msg);
    bool write(const Message &msg);
    bool close();
};