#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "conn_fifo.h"

std::unique_ptr<AbstractConnection> AbstractConnection::createConnection(pid_t pid, bool isHost) {
    return std::make_unique<FifoConnection>(pid, isHost);
}


void FifoConnection::connOpen(size_t id, bool isHost) {
    // Create Fifo file
    if (isHost) {
        if (mkfifo(fifoFilename.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) < 0)
            throw ("Creation error");
    }

    // Open Fifo file
    fileId =::open(fifoFilename.c_str(), O_RDWR);
    if (fileId < 0) {
        throw ("Open error");
    }
}

void FifoConnection::connRead(void* buf, size_t count) {
    if (::read(fileId, buf, count) < 0)
        throw("Read error");
}

void FifoConnection::connWrite(void* buf, size_t count) {
    if (::write(fileId, buf, count) < 0)
        throw("Write error");
}

void FifoConnection::connClose() {
    close(fileId);

    // Close Fifo file
    if (isHost)
        unlink(fifoFilename.c_str());
}

