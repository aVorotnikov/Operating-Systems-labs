#ifndef __CONNECTION_H_
#define __CONNECTION_H_

#include <memory>
#include <sys/types.h>

class Connection {
public:
    static std::unique_ptr<Connection> Create(pid_t clientPid, bool isHost);

    virtual void Read(void* buf, size_t count) = 0;
    virtual void Write(const void* buf, size_t count) = 0;

    virtual ~Connection(void) = default;
};

#endif //!__CONNECTION_H_