#ifndef __CONN_FIFO_H_
#define __CONN_FIFO_H_

#include <string>

#include "connection.h"

class fifo : public Connection {
public:
    fifo(pid_t clientPid, bool isHost);

    void Read(void *buf, size_t count) override;
    void Write(const void *buf, size_t count) override;

    ~fifo(void);

private:
    bool _isHost;
    std::string _fifoName;
    int _fileDescr = -1;
};

#endif //!__CONN_FIFO_H_