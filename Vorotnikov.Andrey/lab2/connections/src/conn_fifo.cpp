#include "../conn_fifo.h"

std::shared_ptr<Connection> GetConnection(pid_t pid, Connection::Type type)
{
    return std::make_shared<ConnectionFifo>(pid, type);
}

ConnectionFifo::ConnectionFifo(std::size_t id, Connection::Type type)
{
}

ConnectionFifo::~ConnectionFifo()
{
}

bool ConnectionFifo::Read(void* buf, const std::size_t count)
{
    return false;
}

bool ConnectionFifo::Write(void* buf, const std::size_t count)
{
    return false;
}
