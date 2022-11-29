#pragma once

#include <sys/shm.h>
#include "abstr_conn.h"

class SegConnection : public AbstractConnection {
private:
    const std::string SEG_CODE = "SEG_CONN";
    const uint SIZE = sizeof(int);

    bool isHost;
    int segId;
    void* seg = nullptr;
public:
    SegConnection(pid_t pid, bool isHost) : isHost(isHost) { TYPE_CODE = SEG_CODE; };

    void connOpen(size_t pid, bool isHost) override;
    void connRead(void* buf, size_t count) override;
    void connWrite(void* buf, size_t count) override;
    void connClose() override;

    ~SegConnection() = default;
};