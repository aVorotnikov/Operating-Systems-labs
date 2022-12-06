#ifndef __CONN_SHM_H_
#define __CONN_SHM_H_

#include <string>

#include "connection.h"

class shm : public Connection {
public:
    shm(pid_t clientPid, bool isHost);

    bool Read(void *buf, size_t count) override;
    bool Write(const void *buf, size_t count) override;

    ~shm(void);

private:
    const size_t _size = 1024;
    bool _isHost;
    std::string _shmName;
    int _fileDescr = -1;
    void *_bufptr = nullptr; 
};

#endif //!__CONN_SHM_H_