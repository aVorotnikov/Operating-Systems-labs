#include "../conn_seg.h"

#include <sys/shm.h>
#include <fcntl.h>

#include <cstring>
#include <stdexcept>

std::shared_ptr<Connection> GetConnection(pid_t pid, Connection::Type type)
{
    return std::make_shared<ConnectionSeg>(pid, type);
}

ConnectionSeg::ConnectionSeg(pid_t pid, Connection::Type type) : Connection(pid, type), shmId_(-1), segPtr_(nullptr)
{
    static constexpr mode_t getshmModeCommon = 0666;

    mode_t mode = getshmModeCommon;
    if (Connection::Type::Host == type_)
        mode |= (IPC_CREAT | O_EXCL);
    shmId_ = shmget(hostPid_, segSize_, mode);
    if (-1 == shmId_)
        throw std::runtime_error("Failed to create segment");

    segPtr_ = shmat(shmId_, 0, 0);
    if (reinterpret_cast<void*>(-1) == segPtr_)
    {
        if (Connection::Type::Host == type_)
            shmctl(shmId_, IPC_RMID, 0);
        throw std::runtime_error("Failed to attach segment");
    }
}

ConnectionSeg::~ConnectionSeg()
{
    shmdt(segPtr_);
    if (Connection::Type::Host == type_)
        shmctl(shmId_, IPC_RMID, 0);
}

bool ConnectionSeg::Read(void* buf, const std::size_t count)
{
    if (count > segSize_)
        return false;
    memcpy(buf, segPtr_, count);
    return true;
}

bool ConnectionSeg::Write(void* buf, const std::size_t count)
{
    if (count > segSize_)
        return false;
    memcpy(segPtr_, buf, count);
    return true;
}
