#pragma once

#include <mqueue.h>
#include "abstr_conn.h"
#include "../messages/messages.h"

class MQConnection : public AbstractConnection {
private:
    const std::string MQ_CODE = "mq";
    const size_t MAX_SIZE = sizeof(Message);

    bool isCreator;
    std::string mqFilename;
    mqd_t mq;

public:
    MQConnection(pid_t pid, bool isHost) : isCreator(isHost) {
        TYPE_CODE = MQ_CODE;
        this->mqFilename = "/mq_" + std::to_string(pid);
    };

    void connReinit() {};

    void connOpen(size_t id, bool isHost) override;
    void connRead(void* buf, size_t count) override;
    void connWrite(void* buf, size_t count) override;
    void connClose() override;

    ~MQConnection() = default;
};