#include <syslog.h>

#include "messages.h"

// Protected queue's methods, which provides sending and recieving messages

void ConnectedQueue::Push(const Message& msg) {
    mutex.lock();
    q.push(msg);
    mutex.unlock();
}

bool ConnectedQueue::Pop(Message* msg) {
    mutex.lock();
    if (q.empty()) {
        mutex.unlock();
        return false;
    }

    *msg = q.front();
    q.pop();
    mutex.unlock();
    return true;
}

bool ConnectedQueue::PushFromConnection(AbstractConnection *conn) {
    mutex.lock();
    uint size = 0;
    
    // Try to get messages count and leave if we should
    try {
        conn->connRead((void *)&size, sizeof(uint));   // Read messages cnt
        if (size == 0) {
            mutex.unlock();
            return true;
        }
    }
    catch (const char *e) {
        syslog(LOG_ERR, "%s", e);
        mutex.unlock();
        return false;
    }

    // Reading messages
    std::string log_str = std::string("Starting pushing msgs from conn_") + conn->getConnectionCode(); 
    syslog(LOG_INFO, log_str.c_str());

    Message msg;
    for (uint i = 0; i < size; i++) {
        msg = {0};
        try {
            conn->connRead((void *)&msg, sizeof(Message));

            std::string log_str = std::string("Readed msg: ") + std::string(msg.text);
            syslog(LOG_INFO, log_str.c_str());
            q.push(msg);
        }
        catch (const char *e) {
            syslog(LOG_ERR, "%s", e);
            mutex.unlock();
            return false;
        }
    }

    mutex.unlock();
    return true;
}

bool ConnectedQueue::PopToConnection(AbstractConnection *conn) {
    mutex.lock();
	if (q.empty()) {
        mutex.unlock();
        return false;
    }

    std::string log_str = std::string("Starting pop to conn_") + conn->getConnectionCode(); 
    syslog(LOG_INFO, log_str.c_str());
    uint size = q.size();
    conn->write((void *)&size, sizeof(uint));   // Write messages cnt
    
    Message msg;
    while (!q.empty()) {
        msg = q.front();
        
        try {
            conn->write((void *)&msg, sizeof(Message));
            q.pop();
        }
        catch (const char *e) {
            syslog(LOG_ERR, "%s", e);
            mutex.unlock();
            return false;
        }
    }

    std::string log_str = std::string("Succesful pop into conn_") + conn->getConnectionCode();     
    syslog(LOG_INFO, log_str.c_str());
    mutex.unlock();
    return true;
}