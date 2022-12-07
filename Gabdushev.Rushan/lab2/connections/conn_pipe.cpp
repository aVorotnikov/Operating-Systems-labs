#include "conn_pipe.h"

#include <sys/syslog.h>

std::unordered_map<std::string, PipePair> pipeDict;

Connection* Connection::createDefault(const std::string &name, bool isHost)
{
    PipePair pipePair;
    auto pipePairIterator = pipeDict.find(name);
    if (pipePairIterator == pipeDict.end())
    {
        int tmp[2];
        if (pipe(tmp) < 0)
        {
            syslog(LOG_ERR, "ERROR: failed to create pipe");
            return nullptr;
        }
        pipePair.hostDesriptors.read = tmp[0];
        pipePair.hostDesriptors.write = tmp[1];
        if (pipe(tmp) < 0)
        {
            syslog(LOG_ERR, "ERROR: failed to create pipe");
            return nullptr;
        }
        pipePair.clientDesriptors.read = tmp[0];
        pipePair.clientDesriptors.write = tmp[1];
        pipeDict[name] = pipePair;
    }
    else
    {
        pipePair = pipePairIterator->second;
    }
    return new Pipe(name, isHost, pipePair);
}

Pipe::Pipe(const std::string &name, bool isHost, const PipePair &pipePair)
{
    this->isHost = isHost;
    this->name = name;
    this->pipePair = pipePair;
}

bool Pipe::open()
{
    if (isHost)
    {
        readDescriptor = pipePair.clientDesriptors.read;
        writeDescriptor = pipePair.hostDesriptors.write;
        if (::close(pipePair.clientDesriptors.write) < 0 || ::close(pipePair.hostDesriptors.read) < 0)
        {
            syslog(LOG_ERR, "ERROR: failed to close");
            return false;
        }
    }
    else
    {
        readDescriptor = pipePair.hostDesriptors.read;
        writeDescriptor = pipePair.clientDesriptors.write;
        if (::close(pipePair.hostDesriptors.write) < 0 || ::close(pipePair.clientDesriptors.read) < 0)
        {
            syslog(LOG_ERR, "ERROR: failed to close");
            return false;
        }
    }
    return true;
}

bool Pipe::read(Message &msg)
{
    if (::read(readDescriptor, &msg, sizeof(Message)) < 0)
    {
        syslog(LOG_ERR, "ERROR: failed to read");
        return false;
    }
    return true;
}

bool Pipe::write(const Message &msg)
{
    if (::write(writeDescriptor, &msg, sizeof(Message)) < 0)
    {
        syslog(LOG_ERR, "ERROR: failed to write");
        return false;
    }
    return true;
}

bool Pipe::close()
{
    if (::close(readDescriptor) < 0 || ::close(writeDescriptor) < 0)
    {
        syslog(LOG_ERR, "ERROR: failed to close");
        return false;
    }
    return true;
}