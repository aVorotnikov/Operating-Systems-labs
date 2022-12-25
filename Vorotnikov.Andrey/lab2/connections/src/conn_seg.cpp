#include "../conn_seg.h"

std::shared_ptr<Connection> GetConnection(pid_t pid, Connection::Type type)
{
    return std::make_shared<ConnectionSeg>(pid, type);
}

ConnectionSeg::ConnectionSeg(std::size_t id, Connection::Type type)
{
}

ConnectionSeg::~ConnectionSeg()
{
}

bool ConnectionSeg::Read(void* buf, const std::size_t count)
{
    return false;
}

bool ConnectionSeg::Write(void* buf, const std::size_t count)
{
    return false;
}
