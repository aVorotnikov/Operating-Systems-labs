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

size_t ConnectedQueue::GetSize() {
    size_t elements = -1;

    mutex.lock();
    elements = q.size();
    mutex.unlock();

    return elements;
}

bool ConnectedQueue::PushFromConnection(AbstractConnection *conn) {
    mutex.lock();
    conn->connReinit();

    uint size = 0;
    
    // Try to get messages count and leave if we should
    try {
        conn->connRead((void *)&size, sizeof(uint));   // Read messages cnt
        if (size == 0) {
            mutex.unlock();
            return true;
        }
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "%s", e.what());
        mutex.unlock();
        return false;
    }
    syslog(LOG_ERR, "Total %u msgs!", size);
    

    // Reading messages
    std::string log_str = std::string("Starting pushing msgs from conn_") + conn->getConnectionCode(); 
    //syslog(LOG_INFO, "%s", log_str.c_str());
    
    Message msg;
    for (uint i = 0; i < size; i++) {
        syslog(LOG_ERR, "Reading msg #%u", i);
        msg = {0};
        try {
            conn->connRead((void *)&msg, sizeof(Message));

            log_str = std::string("Readed msg: ") + std::string(msg.text);
            syslog(LOG_INFO, "%s", log_str.c_str());
            q.push(msg);
        }
        catch (std::exception &e) {
            syslog(LOG_ERR, "%s", e.what());
            mutex.unlock();
            return false;
        }
    }

    mutex.unlock();
    return true;
}

bool ConnectedQueue::PopToConnection(AbstractConnection *conn) {
    mutex.lock();
	conn->connReinit();

    std::string log_str = std::string("Starting pop to conn_") + conn->getConnectionCode(); 
    //syslog(LOG_INFO, "%s", log_str.c_str());
    uint size = q.size();
    conn->connWrite((void *)&size, sizeof(uint));   // Write messages cnt
    
    Message msg;
    while (!q.empty()) {
        msg = q.front();
        
        try {
            conn->connWrite((void *)&msg, sizeof(Message));
            q.pop();
        }
        catch (std::exception &e) {
            syslog(LOG_ERR, "%s", e.what());
            mutex.unlock();
            return false;
        }
    }

    log_str = std::string("Succesful pop into conn_") + conn->getConnectionCode();     
    //syslog(LOG_INFO, "%s", log_str.c_str());
    mutex.unlock();
    return true;
}