#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "conn_shm.h"

std::unique_ptr<Connection> Connection::Create(pid_t clientPid, bool isHost) {
    return std::make_unique<shm>(clientPid, isHost);
}

shm::shm(pid_t clientPid, bool isHost) {
    _isHost = isHost;
    _shmName = "shm_" + std::to_string(clientPid);

    int f = _isHost ? O_CREAT | O_RDWR | O_EXCL : O_RDWR;
    
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

bool shm::Read(void* buf, size_t count) {
    if (count > _size)
        return false;
    memcpy(buf, _bufptr, count);
    return true;
}

bool shm::Write(const void* buf, size_t count) {
    if (count > _size)
        return false;
    memcpy(_bufptr, buf, count);
    return true;
}

shm::~shm(void) {
    munmap(_bufptr, _size);
    close(_fileDescr);
    if (_isHost)
        shm_unlink(_shmName.c_str());
}