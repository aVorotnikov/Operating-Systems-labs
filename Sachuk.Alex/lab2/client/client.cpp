#include <QApplication>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <semaphore.h>
#include <csignal>
#include <thread>

#include "client.h"

void Client::SignalHandler(int signum, siginfo_t* info, void *ptr) {
    switch (signum) {
    case SIGTERM:
        Client::getInstance().isRunning = false;
        return;
    case SIGINT:
        syslog(LOG_INFO, "INFO[Client] client terminated");
        exit(EXIT_SUCCESS);
        return;
    case SIGUSR1:
        syslog(LOG_INFO, "INFO[Client] chat terminated");
        kill(Client::getInstance().hostPid, SIGTERM);
        exit(EXIT_SUCCESS);
        return;
    default:
        syslog(LOG_INFO, "INFO[Client] unknown command");
    }
}

Client::Client() {
    struct sigaction sig{};

    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = Client::SignalHandler;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGINT, &sig, nullptr);
    sigaction(SIGUSR1, &sig, nullptr);
}


bool Client::init(const pid_t& hostPid) {
    // init connections and semafores
    syslog(LOG_INFO, "INFO[Client]: initializing");
    isRunning = connectionPrepare(hostPid);

    if (isRunning)
        syslog(LOG_INFO, "INFO[Client]: All inited succesful");
    else
        syslog(LOG_INFO, "INFO[Client]: Cant init chat");

    return isRunning;
}


void Client::run() {
    // init connections and semafores
    syslog(LOG_INFO, "INFO[Client]: Client started run");
    
    // start connection working in different thread
    std::thread connThread(&Client::connectionWork, this);

    // starts gui
    std::string winName = "Client";
    int argc = 1;
    char* args[] = { (char*)winName.c_str() };
    QApplication app(argc, args);
    ChatWin window(winName, winWrite, winRead, IsRun);
    window.show();
    app.exec();
    
    // stop working
    stop();
    connThread.join();
}


void Client::stop() {
    if (isRunning.load()) {
        syslog(LOG_INFO, "INFO[Client] stop working");
        isRunning = false;
    }
}


Client& Client::getInstance() {
    static Client clientInstance;
    return clientInstance;
}


bool Client::connectionPrepare(const pid_t& hostPid) {
    // init connections and semaphores
    syslog(LOG_INFO, "INFO [Client]: start init connection");
    this->hostPid = hostPid;
    
    conn = AbstractConnection::createConnection(hostPid, false);

    hostSem = sem_open("/Host-sem", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (hostSem == SEM_FAILED) {
        syslog(LOG_ERR, "ERROR [Client]: cant connect to host sem");
        return false;
    }
    clientSem = sem_open("/Client-sem", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (clientSem == SEM_FAILED) {
        sem_close(hostSem);
        syslog(LOG_ERR, "ERROR [Client]: cant connect to client sem");
        return false;
    }

    try {
        conn->connOpen(hostPid, false);
        Client::getInstance().isRunning = true;
        syslog(LOG_INFO, "INFO [Client]: connection initializing complete!");
        return true;
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "ERROR [Client]: %s", e.what());
        sem_close(hostSem);
        sem_close(clientSem);
        return false;
    }
}

void Client::connectionWork() {
    syslog(LOG_INFO, "INFO [Client]: Client starts working");

    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // for host initing
    while (isRunning.load()) {
        // Send all messages
        if (!connectionWriteMsgs())
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(32)); // for host get semaphore to read

        // Get all messages
        if (!connectionReadMsgs())
            break;
    }
    connectionClose();
}


bool Client::connectionReadMsgs() {
    // wait semaphore
    {
        timespec t;

        clock_gettime(CLOCK_REALTIME, &t);

        t.tv_sec += 5;

        int s = sem_timedwait(clientSem, &t);
        if (s == -1)
        {
            syslog(LOG_ERR, "ERROR[Client] Read semaphore timeout");
            isRunning = false;
            return false;
        }
    }

    messagesIn.PushFromConnection(conn.get());
    return true;
}

bool Client::connectionWriteMsgs() {
    //syslog(LOG_ERR, "INFO [Client]: Trying send msgs...");
    bool res = messagesOut.PopToConnection(conn.get());
    sem_post(hostSem);
    return res;
}

void Client::connectionClose() {
    conn->connClose();
    sem_close(hostSem);
    sem_close(clientSem);
    kill(hostPid, SIGTERM);
}

bool Client::IsRun() {
    return Client::getInstance().isRunning.load();
}
    
bool Client::winRead(Message *msg) {
    return Client::getInstance().messagesIn.Pop(msg);
}

void Client::winWrite(Message msg) {
    Client::getInstance().messagesOut.Push(msg);
}
