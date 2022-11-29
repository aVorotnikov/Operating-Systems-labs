#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "conn_seg.h"

// src : https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-shmget-get-shared-memory-segment

std::unique_ptr<AbstractConnection> AbstractConnection::createConnection(pid_t pid, bool isHost) {
    return std::make_unique<SegConnection>(pid, isHost);
}

void SegConnection::connOpen(size_t pid, bool isHost) {
    if (isHost)
        segId = shmget(pid, SIZE, IPC_CREAT | O_EXCL | 0666);
    else
        segId = shmget(pid, SIZE, 0666);

    if (segId == -1)
        throw("error while opening segment");

    seg = shmat(segId, 0, 0);
    if (seg == (void*)-1) {
        if (isHost)
            shmctl(segId, IPC_RMID, 0);
        
        throw("error while opening segment");
    }
}

void SegConnection::connRead(void* buf, size_t count) {
    if (count > SIZE)
        throw "segment reading error";
    memcpy(buf, seg, count);
}

void SegConnection::connWrite(void* buf, size_t count) {
    if (count > SIZE)
        throw "segment writing error";
    memcpy(seg, buf, count);
}

void SegConnection::connClose() {
    shmdt(seg);
    if (isHost)
        shmctl(segId, IPC_RMID, 0);
}
