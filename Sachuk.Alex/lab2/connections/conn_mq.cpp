#include "conn_mq.h"

std::unique_ptr<AbstractConnection> AbstractConnection::createConnection(pid_t pid, bool isHost) {
    return std::make_unique<MQConnection>(pid, isHost);
}

void MQConnection::connOpen(size_t id, bool isHost) {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 30;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    if (isHost)
        mq = mq_open(mqFilename.c_str(), O_CREAT | O_RDWR | O_EXCL, 0644, &attr);
    else
        mq = mq_open(mqFilename.c_str(), O_RDWR);

    if ((mqd_t) - 1 == mq)
        throw("error while opening mqueue");
}

void MQConnection::connRead(void* buf, size_t count) {
    if (count > MAX_SIZE)
        throw("Read error");
    mq_receive(mq, (char *)buf, MAX_SIZE, nullptr);
}

void MQConnection::connWrite(void* buf, size_t count) {
    if (count > MAX_SIZE)
        throw("Write error");
    mq_send(mq, (const char *)buf, count, 0);
}

void MQConnection::connClose() {
    mq_close(mq);
}