#ifndef CLIENT_H
#define CLIENT_H
#include <sys/types.h>
#include <atomic>
#include <vector>
#include <queue>
#include <mutex>
#include <stdlib.h>
#include <string.h>
#include <bits/types/siginfo_t.h>
#include "../gui/gui.h"

class Client {
private:
    // queue of input messages
    std::queue<Message> m_inputMessages;
    std::mutex m_inputMessagesMutex;

    // queue of input messages
    std::queue<Message> m_outputMessages;
    std::mutex m_outputMessagesMutex;

    // client pid
    std::atomic<pid_t> m_hostPid = -1;
    // variable for singleton
    static Client m_clientInstance;
    // atomic bool for terminating
    std::atomic<bool> m_isRunning = true;
    // atomic bool for init work
    std::atomic<bool> m_isHostReady = false;
    // handler for signals
    static void SignalHandler(int signum, siginfo_t* info, void *ptr);
    // Connection working function
    void ConnectionWork(void);
    // Private constructor
    Client(void);
    // Blocked constructors
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
public:

    static Client &GetInstance(void) { return m_clientInstance; }

    void Run(pid_t host_id);

    void Stop(void);

    ~Client();
};

#endif //CLIENT_H
