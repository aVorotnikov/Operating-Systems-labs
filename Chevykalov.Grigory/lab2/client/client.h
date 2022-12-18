#ifndef __CLIENT_H_
#define __CLIENT_H_

#include <sys/types.h>
#include <cstdlib>
#include <atomic>

#include "../connection/connection.h"

class Client {
private:
    const int _maxNum = 100;
    const int _divider = 2;
    const int _timeInter = 5;

    pid_t _hostPid = -1;
    std::atomic<bool> _isTerminated = false;
    bool _isAlive = true;

    std::unique_ptr<Connection> _conn;
    sem_t *_semRead, *_semWrite;

    static void SignalHandler(int sig);

    bool OpenConnection(void);
    bool GetState(void);
    bool SendNum(void);
    void CloseConnection(void);

    Client(void);

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

public:
    static Client &GetInstance(void) noexcept {
        static Client instance;
        return instance;
    }

    int Run(pid_t hostPid) noexcept;

    void Terminate(void) noexcept;

    ~Client(void) = default; 
};

#endif //__CLIENT_H_