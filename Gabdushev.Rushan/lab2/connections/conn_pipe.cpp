#include "conn_pipe.h"

#include <sys/syslog.h>

std::unordered_map<std::string, PipePair> Pipe::pipeDict = std::unordered_map<std::string, PipePair>();

Connection* Connection::createDefault(const std::string &name, bool isHost)
{
    PipePair pipePair;
    auto pipePairIterator = Pipe::pipeDict.find(name);
    if (pipePairIterator == Pipe::pipeDict.end())
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
        Pipe::pipeDict[name] = pipePair;
    }
    else
    {
        pipePair = pipePairIterator->second;
    }
    return new Pipe(name, isHost, pipePair);
}

Pipe::Pipe(const std::string &name, bool isHost, const PipePair &pipePair) : name(name), isHost(isHost), pipePair(pipePair) {}

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