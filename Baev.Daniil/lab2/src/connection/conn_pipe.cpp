#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <unordered_map>

#include "connection.h"

struct PipePair{
    struct PipeDescriptors{
        int _read;
        int _write;
    };
    PipeDescriptors _hostDesriptors;
    PipeDescriptors _clientDesriptors;
};

std::unordered_map<std::string, PipePair> pipeDict;

struct Connection::Impl{
    Impl(pid_t hostPid, int id, bool isHost);

    ~Impl();

    bool _isHost = false;
    std::string _name;
    PipePair _pipePair;
    int _readDescriptor = -1;
    int _writeDescriptor = -1;
};

Connection::Connection(pid_t hostPid, int id, bool isHost) : pImpl{std::make_unique<Impl>(hostPid, id, isHost)}{};

Connection::~Connection() = default;

bool Connection::read(void *buf, size_t count){
    if (::read(pImpl->_readDescriptor, buf, count) < 0)
        return false;
    return true;
}

bool Connection::write(const void *buf, size_t count){
    if (::write(pImpl->_writeDescriptor, buf, count) < 0)
        return false;
    return true;
}

Connection::Impl::Impl(pid_t hostPid, int id, bool isHost){
    _isHost = isHost;
    _name = "pipe_" + std::to_string(hostPid) + std::to_string(id);

    auto pipePairIterator = pipeDict.find(_name);
    if (pipePairIterator == pipeDict.end()){
        int tmp[2];
        if (pipe(tmp) < 0){
            throw("failed to create pipe");
        }
        _pipePair._hostDesriptors._read = tmp[0];
        _pipePair._hostDesriptors._write = tmp[1];
        if (pipe(tmp) < 0){
            throw("failed to create pipe");
        }
        _pipePair._clientDesriptors._read = tmp[0];
        _pipePair._clientDesriptors._write = tmp[1];
        pipeDict[_name] = _pipePair;
    }
    else{
        _pipePair = pipePairIterator->second;
    }

    if (isHost){
        _readDescriptor = _pipePair._clientDesriptors._read;
        _writeDescriptor = _pipePair._hostDesriptors._write;
    }
    else{
        _readDescriptor = _pipePair._hostDesriptors._read;
        _writeDescriptor = _pipePair._clientDesriptors._write;
        if (close(_pipePair._hostDesriptors._write) < 0 || close(_pipePair._clientDesriptors._read) < 0){
            throw("failed to close");
        }
    }
}

Connection::Impl::~Impl(){
    close(_readDescriptor);
    close(_writeDescriptor);
}