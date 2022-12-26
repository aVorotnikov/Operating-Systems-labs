#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>

#include "connection.h"

struct Connection::Impl{
    Impl(pid_t hostPid, int id, bool isHost);

    ~Impl();

    const size_t _size = 1024;
    bool _isHost;
    std::string _shmName;
    int _fileDescr = -1;
    void *_bufptr = nullptr; 
};

Connection::Connection(pid_t hostPid, int id, bool isHost) : pImpl{std::make_unique<Impl>(hostPid, id, isHost)}{};

Connection::~Connection() = default;

bool Connection::read(void *buf, size_t count){
    if (count > pImpl->_size)
        return false;
    memcpy(buf, pImpl->_bufptr, count);
    return true;
}

bool Connection::write(const void *buf, size_t count){
    if (count > pImpl->_size)
        return false;
    memcpy(pImpl->_bufptr, buf, count);
    return true;
}

Connection::Impl::Impl(pid_t hostPid, int id, bool isHost){
    _isHost = isHost;
    _shmName = "shm_" + std::to_string(hostPid) + std::to_string(id);

    int f;
    if (_isHost)
        f = O_CREAT | O_RDWR | O_EXCL;
    else
        f = O_RDWR;
    
    //0666 mode - three level read-open mode
    _fileDescr = shm_open(_shmName.c_str(), f, 0666);
    if (_fileDescr == -1)
        throw "object creation error";

    ftruncate(_fileDescr, _size);

    _bufptr = mmap(0, _size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, _fileDescr, 0);
    if (_bufptr == MAP_FAILED) {
        close(_fileDescr);
        if (_isHost)
            shm_unlink(_shmName.c_str());
        throw "object mapping error";
    }
}

Connection::Impl::~Impl(){
    munmap(_bufptr, _size);
    close(_fileDescr);
    if (_isHost)
        shm_unlink(_shmName.c_str());
}