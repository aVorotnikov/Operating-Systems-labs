#ifndef __CONN_SEG_H_
#define __CONN_SEG_H_

#include <string>

#include "connection.h"

class seg : public Connection
{
public:
    seg(pid_t clientPid, bool isCreator);
    void Open(size_t hostPid, bool isCreator) override;
    void Get(void* buf, size_t count) override;
    void Send(void* buf, size_t count) override;
    void Close(void) override;

    ~seg(void);

private:
    static const int BULK_SIZE = 256;
    int m_shmid;
    void *m_ptr;
};

#endif //!__CONN_SEG_H_
