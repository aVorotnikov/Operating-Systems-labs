#pragma once

#include "conn.h"

#include <sys/socket.h>

class Socket : public Connection
{
private:
    static constexpr char PREFIX[] = "/tmp/wolf_n_goat_socket_conn_";
    static constexpr size_t MAX_CLIENT_NUM = 1;

    socklen_t hostSocket;
    socklen_t clientSocket;

    bool isHost = false;
    std::string name;

public:
    Socket() = delete;
    Socket(const std::string &name, bool isHost);
    bool open();
    bool read(Message &msg);
    bool write(const Message &msg);
    bool close();
};