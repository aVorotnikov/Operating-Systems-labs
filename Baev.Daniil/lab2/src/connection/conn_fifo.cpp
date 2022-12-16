#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>

#include "connection.h"

struct Connection::Impl{
    Impl(pid_t hostPid, int id, bool isHost);

    ~Impl();

    bool _isHost;
    std::string _fifoName;
    int _fileDescr = -1;
};

Connection::Connection(pid_t hostPid, int id, bool isHost) : pImpl{std::make_unique<Impl>(hostPid, id, isHost)}{};

Connection::~Connection() = default;

bool Connection::read(void *buf, size_t count){
    if (::read(pImpl->_fileDescr, buf, count) < 0)
        return false;
    return true;
}

bool Connection::write(const void *buf, size_t count){
    if (::write(pImpl->_fileDescr, buf, count) < 0)
        return false;
    return true;
}

Connection::Impl::Impl(pid_t hostPid, int id, bool isHost){
    _isHost = isHost;
    _fifoName = "/tmp/fifo_" + std::to_string(hostPid) + std::to_string(id);

    if (_isHost)
        if (mkfifo(_fifoName.c_str(), 0666))
            throw "fifo creation error";

    _fileDescr = open(_fifoName.c_str(), O_RDWR);
    if (_fileDescr == -1) {
        if (_isHost)
            unlink(_fifoName.c_str());
        throw "fifo openning error";
    }
}

Connection::Impl::~Impl(){
    close(_fileDescr);
    if (_isHost)
        unlink(_fifoName.c_str());
}