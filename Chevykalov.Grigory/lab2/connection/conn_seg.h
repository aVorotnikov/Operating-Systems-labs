#ifndef __CONN_SEG_H_
#define __CONN_SEG_H_

#include "connection.h"

class seg : public Connection {
public:
    seg(pid_t clientPid, bool isHost);

    bool Read(void *buf, size_t count) override;
    bool Write(const void *buf, size_t count) override;

    ~seg(void);

private:
    const size_t _size = sizeof(int);
    bool _isHost;
    void *_segptr = nullptr;
    int _shmid = -1;
};

#endif //!__CONN_SEG_H_