#include "../conn_mq.h"

std::shared_ptr<Connection> GetConnection(pid_t pid, Connection::Type type)
{
    return std::make_shared<ConnectionMq>(pid, type);
}

ConnectionMq::ConnectionMq(std::size_t id, Connection::Type type)
{
}

ConnectionMq::~ConnectionMq()
{
}

bool ConnectionMq::Read(void* buf, const std::size_t count)
{
    return false;
}

bool ConnectionMq::Write(void* buf, const std::size_t count)
{
    return false;
}
