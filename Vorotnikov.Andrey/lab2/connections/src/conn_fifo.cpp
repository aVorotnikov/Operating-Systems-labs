#include "../conn_fifo.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdexcept>

std::shared_ptr<Connection> GetConnection(pid_t pid, Connection::Type type)
{
    return std::make_shared<ConnectionFifo>(pid, type);
}

ConnectionFifo::ConnectionFifo(pid_t pid, Connection::Type type) :
    Connection(pid, type),
    fileDescriptor_(-1),
    absPath_(filePathTemplate_ + std::to_string(hostPid_))
{
    static constexpr mode_t mkfifoMode = 0777;
    if (Connection::Type::Host == type_)
        if (-1 == mkfifo(absPath_.c_str(), mkfifoMode))
            throw std::runtime_error("Failed to make fifo");
    fileDescriptor_ = open(absPath_.c_str(), O_RDWR);
    if (-1 == fileDescriptor_)
        throw std::runtime_error("Failed to create fifo file");
}

ConnectionFifo::~ConnectionFifo()
{
    close(fileDescriptor_);
}

bool ConnectionFifo::Read(void* buf, const std::size_t count)
{
    return read(fileDescriptor_, buf, count) >= 0;
}

bool ConnectionFifo::Write(void* buf, const std::size_t count)
{
    return write(fileDescriptor_, buf, count) >= 0;
}
