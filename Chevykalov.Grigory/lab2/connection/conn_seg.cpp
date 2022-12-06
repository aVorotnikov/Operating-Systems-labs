#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "conn_seg.h"

std::unique_ptr<Connection> Connection::Create(pid_t clientPid, bool isHost) {
    return std::make_unique<seg>(clientPid, isHost);
}

seg::seg(pid_t clientPid, bool isHost) {
    _isHost = isHost;

    int f = _isHost ? IPC_CREAT | O_EXCL | 0666 : 0666;

    _shmid = shmget(clientPid, _size, f);
    if (_shmid == -1)
        throw "segment creation error";

    _segptr = shmat(_shmid, 0, 0);
    if (_segptr == (void*)-1) {
        if (_isHost)
            shmctl(_shmid, IPC_RMID, 0);
        throw "segment attachment error";
    }
}

bool seg::Read(void* buf, size_t count) {
    if (count > _size)
        return false;
    memcpy(buf, _segptr, count);
    return true;
}

bool seg::Write(const void* buf, size_t count) {
    if (count > _size)
        return false;
    memcpy(_segptr, buf, count);
    return true;
}

seg::~seg(void) {
    shmdt(_segptr);
    if (_isHost)
        shmctl(_shmid, IPC_RMID, 0);
}