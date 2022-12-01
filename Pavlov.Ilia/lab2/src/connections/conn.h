#pragma once

#include <sys/types.h>

class Conn {
public:
    enum class Type {
        HOST,
        CLIENT
    };
    static Conn* GetConn(pid_t hostPid, Type type);
    virtual bool Open() = 0;
    virtual bool Read(void* buf, size_t count) = 0;
    virtual bool Write(void* buf, size_t count) = 0;
    virtual void Close() = 0;
};
