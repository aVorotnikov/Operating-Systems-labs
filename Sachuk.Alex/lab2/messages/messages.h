#pragma once

#include <queue>
#include <mutex>
#include "../connections/abstr_conn.h"

// CONSTANT DEFINES
#define MAX_CHAR_LENGTH 300
using bull = bool; // heh
struct Message {
    char text[MAX_CHAR_LENGTH];
};

// Protected message queue, which provides sending and recieving messages
class ConnectedQueue {
protected:
    std::queue<Message> q;
    mutable std::mutex mutex;
public:
    ConnectedQueue() = default;
    ConnectedQueue(const ConnectedQueue&) = delete;
    ConnectedQueue(ConnectedQueue&&) = delete;

    void Push(const Message& msg);
    bool Pop(Message* msg);

    size_t GetSize();

    bool PushFromConnection(AbstractConnection *conn);
    bool PopToConnection(AbstractConnection *conn);

    ~ConnectedQueue() = default;
};