#pragma once

#include <atomic>
#include <mutex>
#include <sys/types.h>
#include <semaphore.h>   
#include <bits/types/siginfo_t.h>

#include "../client/client.h"
#include "../window/window.h"
#include "../connections/abstr_conn.h"
#include "../messages/messages.h"


// 'Host' managment class: singleton
class Host {
private:
    static Host hostInstance;   // Instance

    Client client;
    std::atomic<bool> isRunning = true;

    // Signals and msgs managment
    ConnectedQueue messagesIn;
    ConnectedQueue messagesOut;

    static void SignalHandler(int signum, siginfo_t* info, void *ptr);
    void connectionWork();
    bool connectionPrepare(AbstractConnection **conn, sem_t **sem_read, sem_t **sem_write);
    bool connectionReadMsgs(AbstractConnection *conn, sem_t *sem_read, sem_t *sem_write);
    bool connectionWriteMsgs(AbstractConnection *conn, sem_t *sem_read, sem_t *sem_write);
    void connectionClose(AbstractConnection *conn, sem_t *sem_read, sem_t *sem_write);

    // Window managment
    Window *window = nullptr;
    
    static bool winRead(Message *msg);
    static void winWrite(Message msg);

    // constructions 
    Host();
    Host(const Host&) = delete;
    Host(Host&&) = delete;
public:
    static const Host& getInstance();
    void run();
    void stop();

    ~Host();
};