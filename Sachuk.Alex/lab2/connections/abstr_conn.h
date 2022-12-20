#pragma once

#include <memory>
#include <string>

class AbstractConnection {
protected:
    std::string TYPE_CODE;
public:
    static std::unique_ptr<AbstractConnection> createConnection(pid_t pid, bool isHost);
    
    virtual void connOpen(size_t id, bool create) = 0;
    virtual void connRead(void* buf, size_t count) = 0;
    virtual void connWrite(void* buf, size_t count) = 0;
    virtual void connClose() = 0;
    
    std::string getConnectionCode() { return TYPE_CODE; };
};