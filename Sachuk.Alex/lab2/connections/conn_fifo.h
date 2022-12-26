#pragma once

#include "abstr_conn.h"

class FifoConnection : public AbstractConnection {
private:
    const std::string FIFO_CODE = "fifo";

    bool isHost;
    std::string fifoFilename;
    int fileId;
public:
    FifoConnection(pid_t pid, bool isHost) : isHost(isHost) {
        TYPE_CODE = FIFO_CODE;
        this->fifoFilename = "/tmp/fifo_" + std::to_string(pid);
    };

    void connReinit() {};

    void connOpen(size_t id, bool isHost) override;
    void connRead(void* buf, size_t count) override;
    void connWrite(void* buf, size_t count) override;
    void connClose() override;
    
    ~FifoConnection() = default;
};