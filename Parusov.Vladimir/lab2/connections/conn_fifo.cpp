#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "conn_fifo.h"

Connection * Connection::CreateConnection(pid_t clientPid, bool isHost)
{
    fifo *fif = new fifo(clientPid, isHost);

    if (isHost && mkfifo(fif->GetName().c_str(), S_IFIFO | S_IRWXG | S_IRWXU | S_IRWXO))
    {
        delete fif;
        return nullptr;
    }

    return fif;
}

fifo::fifo(pid_t clientPid, bool isHst)
{
    m_isHost = isHst;
    m_fifoName = "/tmp/fifo_" + std::to_string(clientPid);
}

void fifo::Open(size_t hostPid, bool isCreator)
{
}

void fifo::Get(void* buf, size_t count)
{
    m_file = open(m_fifoName.c_str(), O_RDONLY | O_NONBLOCK);
    if (read(m_file, buf, count) < 0)
    {
        throw ("fifo reading error");
    }
    close(m_file);
}

void fifo::Send(void* buf, size_t count)
{
    m_file = open(m_fifoName.c_str(), O_WRONLY);
    if (write(m_file, buf, count) < 0)
    {
        throw ("fifo writing error");
    }
    close(m_file);
}

void fifo::Close(void)
{
//    close(m_fileWrite);
//    close(m_fileRead);
    /*if (m_isHost)
    {
        close(m_fileWrite);
    }
    else
    {
        close(m_fileRead);
    }*/
}

fifo::~fifo(void)
{
}

