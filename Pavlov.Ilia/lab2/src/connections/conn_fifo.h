#pragma once

#include "conn.h"

#include <string>

class ConnFifo : public Conn {
private:
    pid_t hostPid;
    Type type;
    int fd;
    std::string path;
public:
    ConnFifo(pid_t hostPid, Type type);
    virtual bool Open() override;
    virtual bool Read(void* buf, size_t count) override;
    virtual bool Write(void* buf, size_t count) override;
    virtual void Close() override;
    ~ConnFifo() = default;
};
