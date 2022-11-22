#include <sys/ipc.h>
#include <sys/modes.h>

#include "conn_seg.h"

// src : https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-shmget-get-shared-memory-segment

AbstractConnection * AbstractConnection::createConnection(pid_t pid, bool isHost) {
    return new SegConnection(pid, isHost);
}

void SegConnection::connOpen(size_t id, bool create) {
    if (isHost)
        if ((segId = shmget(IPC_CREAT, SEGMENT_SIZE, 0)) < 0)
            throw("Open error");
    

}

void SegConnection::connRead(void* buf, size_t count) {

}

void SegConnection::connWrite(void* buf, size_t count) {

}

void SegConnection::connClose() {
}
