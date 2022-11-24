#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "conn_fifo.h"

std::unique_ptr<Connection> Connection::Create(pid_t clientPid, bool isHost) {
    return std::make_unique<fifo>(clientPid, isHost);
}

fifo::fifo(pid_t clientPid, bool isHost) {
    _isHost = isHost;
    _fifoName = "/tmp/fifo_" + std::to_string(clientPid);

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

void fifo::Read(void* buf, size_t count) {
    if (read(_fileDescr, buf, count) < 0)
        throw "fifo reading error";
}

void fifo::Write(const void* buf, size_t count) {
    if (write(_fileDescr, buf, count) < 0)
        throw "fifo writing error";
}

fifo::~fifo(void) {
    close(_fileDescr);
    if (_isHost)
        unlink(_fifoName.c_str());
}