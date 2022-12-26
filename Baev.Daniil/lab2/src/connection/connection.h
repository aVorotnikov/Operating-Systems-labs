#pragma once

#include <memory>
#include <sys/types.h>

class Connection{
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
public:
    bool read(void* buf, size_t count);
    bool write(const void* buf, size_t count);

    Connection(pid_t hostPid, int id, bool isHost);
    ~Connection();
};