#include <QApplication>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <semaphore.h>
#include <csignal>
#include <thread>

#include "host.h"

Host::Host() {
    struct sigaction sig{};
    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = Host::SignalHandler;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGINT, &sig, nullptr);
}

void Host::SignalHandler(int signum, siginfo_t* info, void *ptr) {
    switch (signum) {
    case SIGTERM:
        Host::getInstance().isRunning = false;
        return;
    case SIGINT:
        syslog(LOG_INFO, "host terminate");
        exit(EXIT_SUCCESS);
        return;
    default:
        syslog(LOG_INFO, "unknown command");
    }
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
    std::string winName = "Host";
    int argc = 1;
    char* args[] = { (char*)winName.c_str()};
    QApplication app(argc, args);
    ChatWin window(winName, winWrite, winRead, IsRun);
    window.show();
    app.exec();
    
    // stop working
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
    static Host hostInstance;
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
    if (clientSem == SEM_FAILED) {
        sem_close(hostSem);
        syslog(LOG_ERR, "ERROR: client semaphore not created");
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
        Host::getInstance().isRunning = true;
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
    
    while (isRunning.load()) {
        double minutes_passed = std::chrono::duration_cast<std::chrono::minutes>(
            std::chrono::high_resolution_clock::now() - clock).count();

        if (minutes_passed >= 1) {
          syslog(LOG_INFO, "Killing client for 1 minute silence");
          kill(clientPid, SIGTERM);
          clientPid = -1;
          break;
        }


        // Send all messages
        if (!connectionWriteMsgs())
          break;

        // Get all messages
        if (!connectionReadMsgs())
          break;

        std::this_thread::sleep_for(std::chrono::milliseconds(30)); // for client get semaphore
    }
    connectionClose();
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
    syslog(LOG_ERR, "Trying send msgs...");
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
    return Host::getInstance().isRunning.load();
}
    
bool Host::winRead(Message *msg) {
    return Host::getInstance().messagesIn.Pop(msg);
}

void Host::winWrite(Message msg) {
    Host::getInstance().messagesOut.Push(msg);
}
