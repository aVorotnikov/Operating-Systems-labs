#pragma once

#include <sys/shm.h>
#include "abstr_conn.h"
#include "../messages/messages.h"

class SegConnection : public AbstractConnection {
private:
    const std::string SEG_CODE = "shm";
    const uint SIZE = 1024;

    bool isHost;
    int segId;
    void* seg = nullptr;

    int seg_shift;
public:
    SegConnection(pid_t pid, bool isHost) : isHost(isHost) { TYPE_CODE = SEG_CODE; };

    void connOpen(size_t pid, bool isHost) override;
    void connRead(void* buf, size_t count) override;
    void connWrite(void* buf, size_t count) override;
    void connClose() override;

    ~SegConnection() = default;
};