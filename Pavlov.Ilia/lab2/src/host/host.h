#pragma once

#include "gui/mainwindow.h"
#include "../connections/conn.h"
#include "../state.h"

#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include <signal.h>
#include <semaphore.h>

class Host {
private:
    static Host instance;
    pid_t pid = getpid();
    pid_t clientPid = -1;
    std::unique_ptr<Conn> conn;
    State clientState = State::ALIVE;
    static constexpr int diffForAlive = 70;
    static constexpr int diffForDead = 20;
    static constexpr int timeout = 5;
    MainWindow* mw = nullptr;
    sem_t* semRead;
    sem_t* semWrite;

    Host();
    void ConnectClient(pid_t clientPid);
    bool StartRound();
    void StopClient();
    void Terminate(int status);
    friend void SignalHandler(int signum, siginfo_t *si, void *data);
    bool GetClientNum(int* clientNum);
    bool SendState(State);
public:
    static Host* GetInstance();
    pid_t GetPid();
    void SetWindow(MainWindow* window);
    void StartGame();
    ~Host();
};
