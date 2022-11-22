#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "conn_fifo.h"

AbstractConnection * AbstractConnection::createConnection(pid_t pid, bool isHost) {
    return new FifoConnection(pid, isHost);
}


void FifoConnection::connOpen(size_t id, bool create) {
    // Create Fifo file
    if (isHost) {
        unlink(fifoFilename.c_str());
        if (mkfifo(fifoFilename.c_str(), 0666))
            throw ("Creation error");
    }

    // Open Fifo file
    fileId =::open(fifoFilename.c_str(), O_RDWR);
    if (fileId == -1)
        throw ("Open error");
}

void FifoConnection::connRead(void* buf, size_t count) {
    if (read(fileId, buf, count) < 0)
        throw("Read error");
}

void FifoConnection::connWrite(void* buf, size_t count) {
    if (write(fileId, buf, count) < 0)
        throw("Write error");
}

void FifoConnection::connClose() {
    close(fileId);

    // Create Fifo file
    if (isHost)
        unlink(fifoFilename.c_str());
}

