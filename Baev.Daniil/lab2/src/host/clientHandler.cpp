#include <cstdlib>
#include <semaphore.h>
#include <sys/syslog.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <csignal>

#include "clientHandler.h"

bool ClientHandler::forkClient(int id, std::shared_ptr<GameState> gst){
    pid_t hostPid = getpid();
    _gst = gst;
    auto goat = std::make_shared<Goat>();
    _gst->goats.push(goat);
    _id = id;
    if (!openSemaphores(hostPid, id)){
        _gst->goats[_id]->clientStatus = DISCONNECTION;
        _isOpen = false;
        return true;
    };
    
    if (!openConnection(hostPid, id)){
        _gst->goats[_id]->clientStatus = DISCONNECTION;
        _isOpen = false;
        return true;
    }
    _isOpen = true;
    int seed = std::rand();
    _clientPid = fork();
    if (_clientPid == 0) {
        syslog(LOG_INFO, "Start client %d", int(getpid()));
        Client client;

        if (client.init(hostPid, id, seed)) {
            client.run();
            syslog(LOG_INFO, "End client run");
        }
        else {
            syslog(LOG_ERR, "ERROR: client initialization error");
        }
        return false;
    }
    else {
        syslog(LOG_INFO, "Fork client %d", int(_clientPid));
        _gst->goats[_id]->clientStatus = CONNECTION;
        _gst->goats[_id]->state = ALIVE;
        _gst->aliveNumber++;
        return true;
    }
}

void ClientHandler::run(){
    while (_isOpen){
        if (_isNeedToSendRequest.load()){
            _gst->goats[_id]->clientStatus = LOAD;
            if (!sendState(_gst->goats[_id]->state.load())){
                break;
            }
            int number;
            if (!getNum(&number))
                break;
            else {
                _gst->goats[_id]->number = number;
                int delta = _gst->wolfNumber.load() - number;
                delta = delta >= 0 ? delta : -delta;
                if (_gst->goats[_id]->state == ALIVE)
                    if (delta <= int(70 / _gst->goats.len())){
                        _gst->goats[_id]->state = ALIVE;
                    }
                    else {
                        _gst->aliveNumber--;
                        _gst->deadNumber++;
                        _gst->goats[_id]->state = DEAD;
                    }
                else
                    if (delta <= int(20 / _gst->goats.len())){
                        _gst->aliveNumber++;
                        _gst->deadNumber--;
                        _gst->goats[_id]->state = ALIVE;
                    }
                    else {
                        _gst->goats[_id]->state = DEAD;
                    }
            }
            _gst->goats[_id]->clientStatus = CONNECTION;
            _isNeedToSendRequest = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    //kill(_clientPid.load(), SIGTERM);
    //waitpid(_clientPid.load(), 0, 0);
    syslog(LOG_INFO, "End thread");
}

bool ClientHandler::openSemaphores(pid_t hostPid, int id){
    std::string semNameRead = "/host_" + std::to_string(hostPid) + std::to_string(id);
    _hostSemaphore = sem_open(semNameRead.c_str(), O_CREAT | O_EXCL, 0777, 0);
    if (_hostSemaphore == SEM_FAILED) {
        syslog(LOG_ERR, "Semaphore creation error %s", semNameRead.c_str());
        return false;
    }

    std::string semNameWrite = "/client_" + std::to_string(hostPid) + std::to_string(id);
    _clientSemaphore = sem_open(semNameWrite.c_str(), O_CREAT | O_EXCL, 0777, 0);
    if (_clientSemaphore == SEM_FAILED) {
        syslog(LOG_ERR, "Semaphore creation error %s", semNameWrite.c_str());
        sem_close(_hostSemaphore);
        return false;
    }

    syslog(LOG_INFO, "Semaphore created %s %s", semNameRead.c_str(), semNameWrite.c_str());
    return true;
}

bool ClientHandler::openConnection(pid_t hostPid, int id){
    syslog(LOG_INFO, "Creating connection %d", id);
    try {
        _conn = std::make_unique<Connection>(hostPid, id, true);
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "Connection creation error %d", id);
        _clientPid = -1;
        return false;
    }
    catch (const char *e) {
        syslog(LOG_ERR, "Connection creation error %d", id);
        _clientPid = -1;
        return false;
    }
    syslog(LOG_INFO, "Connection opened %d", id);
    return true;
}

pid_t ClientHandler::getPid(void){
    return _clientPid.load();
}

void ClientHandler::close(void){
    sem_close(_hostSemaphore);
    sem_close(_clientSemaphore);
    _isOpen = false;
}

void ClientHandler::sendRequest(void){
    _isNeedToSendRequest = true;
}

bool ClientHandler::getNum(int *num) {
    timespec t;

    clock_gettime(CLOCK_REALTIME, &t);

    t.tv_sec += _timeInter;

    int s = sem_timedwait(_hostSemaphore, &t);
    if (s == -1) {
        syslog(LOG_ERR, "Read semaphore timeout");
        _gst->goats[_id]->clientStatus = DISCONNECTION;
        kill(_clientPid, SIGTERM);
        if(_gst->goats[_id]->state == ALIVE)
            _gst->aliveNumber--;
        else
            _gst->deadNumber--;
        return false;
    }

    if (_conn->read(num, sizeof(int))) {
        return true;
    }
    else {
        syslog(LOG_ERR, "Connection reading error");
        _gst->goats[_id]->clientStatus = DISCONNECTION;
        kill(_clientPid, SIGTERM);
        if(_gst->goats[_id]->state == ALIVE)
            _gst->aliveNumber--;
        else
            _gst->deadNumber--;
        return false;
    }
}

bool ClientHandler::sendState(bool st) {
    if (_conn->write(&st, sizeof(bool))) {
        sem_post(_clientSemaphore);
        return true;
    }
    else {
        syslog(LOG_ERR, "Connection writing error");
        _gst->goats[_id]->clientStatus = DISCONNECTION;
        kill(_clientPid, SIGTERM);
        if(_gst->goats[_id]->state == ALIVE)
            _gst->aliveNumber--;
        else
            _gst->deadNumber--;
        return false;
    }
}