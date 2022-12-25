#include "../conn_mq.h"

#include <stdexcept>

std::shared_ptr<Connection> GetConnection(pid_t pid, Connection::Type type)
{
    return std::make_shared<ConnectionMq>(pid, type);
}

ConnectionMq::ConnectionMq(pid_t pid, Connection::Type type) :
    Connection(pid, type),
    absPath_(filePathTemplate_ + std::to_string(hostPid_)),
    mqd_(-1)
{
    static constexpr mode_t mkopenMode = 0777;
    if (Connection::Type::Host == type_)
    {
        struct mq_attr mqAttr;
        mqAttr.mq_flags = 0;
        mqAttr.mq_maxmsg = 1;
        mqAttr.mq_msgsize = msgSize;
        mqAttr.mq_curmsgs = 0;
        mqd_ = mq_open(absPath_.c_str(), O_CREAT | O_RDWR, mkopenMode, &mqAttr);
    }
    else
        mqd_ = mq_open(absPath_.c_str(), O_RDWR);
    if (-1 == mqd_)
        throw std::runtime_error("Failed to create mq connection");
}

ConnectionMq::~ConnectionMq()
{
    mq_close(mqd_);
}

bool ConnectionMq::Read(void* buf, const std::size_t count)
{
    return -1 != mq_receive(mqd_, static_cast<char*>(buf), msgSize, nullptr);
}

bool ConnectionMq::Write(void* buf, const std::size_t count)
{
    return -1 != mq_send(mqd_, static_cast<char*>(buf), count, 0);
}
