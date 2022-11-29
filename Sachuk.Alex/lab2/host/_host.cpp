#include <sys/syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <semaphore.h>
#include <csignal>
#include <thread>

#include "host.h"

void Host::SignalHandler(int signum, siginfo_t* info, void *ptr) {
    switch (signum) {
    case SIGTERM:
        hostInstance.isRunning = false;
        return;
    case SIGINT:
        syslog(LOG_INFO, "host terminate");
        exit(EXIT_SUCCESS);
        return;
    default:
        syslog(LOG_INFO, "unknown command");
    }
}

Host::Host() {
    struct sigaction sig{};
    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = Host::SignalHandler;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGINT, &sig, nullptr);
}


void Host::run() {
    // init connections and semafores
    syslog(LOG_INFO, "Host-chat started");
    if (connectionPrepare() == false) {
        stop();
        return;
    }
    // start connection working in different thread
    std::thread connThread(&Host::connectionWork, this);

    // starts gui
    window = new Window(std::string("Chat: host window"), winWrite, winRead, IsRun);
    window->run();
    
    // stop working
    delete window;
    stop();
    connThread.join();
}


void Host::stop() {
    if (isRunning.load()) {
        syslog(LOG_INFO, "Chat-host: stop working");
        isRunning = false;
    }
}


Host& Host::getInstance() {
    return hostInstance;
}


bool Host::connectionPrepare() {
    // init connections and semaphores
    syslog(LOG_INFO, "Chat-host-conn: start init");
    hostPid = getpid();
    conn = AbstractConnection::createConnection(hostPid, true);
    hostSem = sem_open("Host-sem", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (hostSem == SEM_FAILED) {
        syslog(LOG_ERR, "ERROR: host semaphore not created");
        return false;
    }
    clientSem = sem_open("Client-sem", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (hostSem == SEM_FAILED) {
        sem_close(hostSem);
        syslog(LOG_ERR, "ERROR: host semaphore not created");
        return false;
    }

    // init child process
    pid_t childPid = fork();
    if (childPid == 0)
    {
        clientPid = getpid();

        if (Client::getInstance().init(hostPid))
            Client::getInstance().run();
        else
        {
            syslog(LOG_ERR, "ERROR: client initialization error");
            return false;
        }
        exit(EXIT_SUCCESS);
    }

    try {
        conn->connOpen(hostPid, true);
        hostInstance.isRunning = true;
        syslog(LOG_INFO, "INFO: host initialize successfully");
        return true;
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "ERROR: %s", e.what());
        sem_close(hostSem);
        sem_close(clientSem);
        return false;
    }
}

void Host::connectionWork() {
    auto clock = std::chrono::high_resolution_clock::now();
    window->setConnected(true);
    
    while (isRunning.load()) {
        double minutes_passed = std::chrono::duration_cast<std::chrono::minutes>(
            std::chrono::high_resolution_clock::now() - clock).count();

        if (minutes_passed >= 1) {
          syslog(LOG_INFO, "Killing client for 1 minute silence");
          kill(m_clientPid, SIGTERM);
          m_clientPid = -1;
          break;
        }

        // Get all messages
        if (!connectionReadMsgs())
          break;

        // Send all messages
        if (!connectionWriteMsgs())
          break;

        std::this_thread::sleep_for(std::chrono::milliseconds(30)); // for client get semaphore
    }
    connectionClose();
    window->setConnected(false);
}


bool Host::connectionReadMsgs() {
    // wait semaphore
    {
        timespec t;

        clock_gettime(CLOCK_REALTIME, &t);

        t.tv_sec += 5;

        int s = sem_timedwait(hostSem, &t);
        if (s == -1)
        {
            syslog(LOG_ERR, "Read semaphore timeout");
            isRunning = false;
            return false;
        }
    }

    messagesIn.PopToConnection(conn.get());
    return true;
}

bool Host::connectionWriteMsgs() {
    bool res = messagesOut.PushFromConnection(conn.get());
    sem_post(clientSem);
    return res;
}

void Host::connectionClose() {
    conn->connClose();
    sem_close(hostSem);
    sem_close(clientSem);
}

bool Host::IsRun() {
    return hostInstance.isRunning.load();
}
    
bool Host::winRead(Message *msg) {
    return hostInstance.messagesIn.Pop(msg);
}

void Host::winWrite(Message msg) {
    hostInstance.messagesOut.Push(msg);
}
