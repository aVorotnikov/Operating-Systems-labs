#include <sys/syslog.h>
#include <csignal>
#include <ctime>
#include <thread>
#include <chrono>
#include <string>
#include <semaphore.h>
#include <fcntl.h>

#include "client.h"
#include "../connection/connection.h"

bool Client::_isTerminated = false;

Client::Client(void) {
    std::signal(SIGTERM, signalHandler);
}

void Client::signalHandler(int sig) {
    switch (sig) {
        case SIGTERM:
            Client::terminate();
            break;
        default:
            syslog(LOG_ERR, "Unknown signal");
    }
}

void Client::terminate(void) noexcept {
    if (!_isTerminated) {
        _isTerminated = true;
        syslog(LOG_INFO, "Terminating client");
    }
}

bool Client::init(pid_t hostPid, int id, int seed){
    std::srand(seed);
    Client::_isTerminated = false;
    _hostPid = hostPid;
    _id = id;
    bool isOpen = openConnection();
    return isOpen;
}

bool Client::openConnection(void) {
    syslog(LOG_INFO, "Creating connection for %d", int(_hostPid));
    try {
        _conn = std::make_unique<Connection>(_hostPid, _id, false);
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "Connection creation error");
        return false;
    }
    catch (const char *e) {
        syslog(LOG_ERR, "Connection creation error");
        return false;
    }

    std::string semNameRead = "/client_" + std::to_string(_hostPid) + std::to_string(_id);
    _clientSemaphore = sem_open(semNameRead.c_str(), 0);
    if (_clientSemaphore == SEM_FAILED) {
        syslog(LOG_ERR, "Semaphore creation error");
        return false;
    }

    std::string semNameWrite = "/host_" + std::to_string(_hostPid) + std::to_string(_id);
    _hostSemaphore = sem_open(semNameWrite.c_str(), 0);
    if (_hostSemaphore == SEM_FAILED) {
        syslog(LOG_ERR, "Semaphore creation error");
        sem_close(_clientSemaphore);
        return false;
    }

    syslog(LOG_INFO, "Connection opened");
    return true;
}

void Client::run(void){
    syslog(LOG_INFO, "Client run %i\n", getpid());
    while (!_isTerminated){
        if (!getState()){
            break;
        }
        if (!sendNum()){
            break;
        }
    }
    closeConnection();
}

bool Client::getState(void){
    timespec t;

    clock_gettime(CLOCK_REALTIME, &t);

    t.tv_sec += _timeInter;

    int s = sem_timedwait(_clientSemaphore, &t);
    if (s == -1) {
        syslog(LOG_ERR, "Read semaphore timeout");
        return false;
    }

    if (_conn->read(&_isAlive, sizeof(bool))) {
        return true;
    }
    else {
        syslog(LOG_ERR, "Connection reading error");
        return false;
    }
}

bool Client::sendNum(void) {
    int num = std::rand() % (_isAlive ? _numAlive: _numDead) + 1;

    if (_conn->write(&num, sizeof(int))) {
        sem_post(_hostSemaphore);
        return true;
    }
    else {
        syslog(LOG_ERR, "Connection writing error");
        return false;
    }
}

void Client::closeConnection(void) {
    sem_close(_clientSemaphore);
    sem_close(_hostSemaphore);
}