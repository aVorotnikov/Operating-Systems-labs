#include <syslog.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "conn_seg.h"

// src : https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-shmget-get-shared-memory-segment

using uint_t = unsigned int;

std::unique_ptr<AbstractConnection> AbstractConnection::createConnection(pid_t pid, bool isHost) {
    return std::make_unique<SegConnection>(pid, isHost);
}

void SegConnection::connOpen(size_t pid, bool isHost) {
    if (isHost)
        segId = shmget(pid, SIZE, IPC_CREAT | O_EXCL | 0666);
    else
        segId = shmget(pid, SIZE, 0666);

    if (segId == -1)
        //throw("error while opening segment");
        syslog(LOG_INFO, "error while opening segment");

    seg = shmat(segId, 0, 0);
    if (seg == (void*)-1) {
        if (isHost)
            shmctl(segId, IPC_RMID, 0);
        
        //throw("error while opening segment");
        syslog(LOG_INFO, "error while opening segment-2");
    }
}

void SegConnection::connRead(void* buf, size_t count) {
    // "count == sizeof(uint_t)" means that we want to know msgs cnt -> starts new reading
    if (count == sizeof(uint_t))
        seg_shift = 0;

    if (count + seg_shift > SIZE)
        throw("segment reading error");
        
    memcpy(buf, ((char *)seg + seg_shift), count);
    seg_shift += count;
}

void SegConnection::connWrite(void* buf, size_t count) {
    // "count == sizeof(uint_t)" means that we want to know msgs cnt -> starts new writting
    if (count == sizeof(uint_t))
        seg_shift = 0;

    if (count + seg_shift > SIZE)
        throw "segment writting error";
        
    memcpy(((char *)seg + seg_shift), buf, count);
    seg_shift += count;
}

void SegConnection::connClose() {
    shmdt(seg);
    if (isHost)
        shmctl(segId, IPC_RMID, 0);
}
