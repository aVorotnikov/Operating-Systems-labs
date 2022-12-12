#pragma once

#include <sys/types.h>
#include <semaphore.h>
#include <cstdlib>

#include "../connection/connection.h"

class Client {
public:
    const int _numAlive = 100;
    const int _numDead = 50;
    const int _timeInter = 5;
    Client(void);

    bool init(pid_t hostPid, int id, int seed);
    void run(void);

    static void terminate(void) noexcept;

    ~Client(void) = default; 

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
private:
    pid_t _hostPid = -1;
    int _id;
    static bool _isTerminated;
    bool _isAlive = true;

    std::unique_ptr<Connection> _conn;
    sem_t* _hostSemaphore;
    sem_t* _clientSemaphore;

    static void signalHandler(int sig);

    bool openConnection(void);
    bool getState(void);
    bool sendNum(void);
    void closeConnection(void);
};