#include <fcntl.h>
#include <sys/shm.h>
#include <string.h>

#include "conn_seg.h"

Connection * Connection::CreateConnection(pid_t clientPid, bool isHost)
{
    seg *se = new seg(clientPid, isHost);

    return se;
}

seg::seg(pid_t clientPid, bool isCreator)
{
    if (isCreator)
        m_shmid = shmget(clientPid, BULK_SIZE, IPC_CREAT | 0660);
    else
        m_shmid = shmget(clientPid, BULK_SIZE, 0);
}

void seg::Open(size_t hostPid, bool isCreator)
{
    m_ptr = shmat(m_shmid, nullptr, 0);
}

void seg::Get(void* buf, size_t count)
{
    if (count > BULK_SIZE)
        throw("Too much");
    memcpy(buf, m_ptr, count);
}

void seg::Send(void* buf, size_t count)
{
    if (count > BULK_SIZE)
        throw("Too much");
    memcpy(m_ptr, buf, count);
}

void seg::Close(void)
{
}

seg::~seg(void)
{
}

