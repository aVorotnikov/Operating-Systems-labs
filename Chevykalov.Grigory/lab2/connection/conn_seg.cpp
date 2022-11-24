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

    int f;
    if (_isHost)
        f = IPC_CREAT | O_EXCL | 0666;
    else
        f = 0666;

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

void seg::Read(void* buf, size_t count) {
    if (count > _size)
        throw "segment reading error";
    memcpy(buf, _segptr, count);
}

void seg::Write(const void* buf, size_t count) {
    if (count > _size)
        throw "segment writing error";
    memcpy(_segptr, buf, count);
}

seg::~seg(void) {
    shmdt(_segptr);
    if (_isHost)
        shmctl(_shmid, IPC_RMID, 0);
}