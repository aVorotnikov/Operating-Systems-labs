#pragma once

#include <memory>
#include <atomic>
#include <sys/syslog.h>
#include <mutex>

#include "../client/client.h"
#include "../gui/model.hpp"

class ClientHandler{
public:
    pid_t getPid(void);

    bool forkClient(int id, std::shared_ptr<GameState> gst);
    void run();
    void sendRequest(void);

    void close(void);

    ClientHandler() = default;

    ClientHandler(const ClientHandler&) = default;
    ClientHandler& operator=(const ClientHandler&) = default;

    ClientHandler (ClientHandler&&) = default;
    ClientHandler& operator = (ClientHandler&&) = default;

    ~ClientHandler(){
        syslog(LOG_INFO, "Destroy Client Handler");
    }
private:
    const int _timeInter = 5;
    std::atomic<pid_t> _clientPid = -1;
    std::atomic<bool> _isNeedToSendRequest = false;
    std::atomic<bool> _isOpen = false;
    std::atomic<int> _id;

    sem_t* _hostSemaphore;
    sem_t* _clientSemaphore;

    std::unique_ptr<Connection> _conn;
    std::shared_ptr<GameState> _gst;

    bool openSemaphores(pid_t hostPid, int id);
    bool openConnection(pid_t hostPid, int id);
    bool getNum(int *num);
    bool sendState(bool st);
};