#include "client.h"

#include <iostream>
#include <syslog.h>
#include <signal.h>
#include <cstdlib>
#include <semaphore.h>
#include <fcntl.h>

Client::Client(pid_t hostPid) : hostPid(hostPid), conn(Conn::GetConn(hostPid, Conn::Type::CLIENT)) {
    openlog("client", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);

    std::string semReadPath = "/sem2" + std::to_string(hostPid);
    semRead = sem_open(semReadPath.c_str(), 0);
    if (semRead == SEM_FAILED) {
        syslog(LOG_ERR, "Could not open semaphore. Terminate");
        exit(EXIT_FAILURE);
    }

    std::string semWritePath = "/sem1" + std::to_string(hostPid);
    semWrite = sem_open(semWritePath.c_str(), 0);
    if (semWrite == SEM_FAILED) {
        syslog(LOG_ERR, "Could not open semaphore. Terminate");
        exit(EXIT_FAILURE);
    }

    srand(time(nullptr));
}

bool Client::OpenConnection() {
    time_t t1 = time(nullptr);
    time_t t2;
    bool conn_rc = false;
    syslog(LOG_INFO, "Try open connection");
    do {
        conn_rc = conn->Open();
        t2 = time(nullptr);
    } while (!conn_rc && difftime(t1, t2) < connTimeout);
    return conn_rc;
}

static int _random(int a, int b) {
  return int((1.0*rand() + 1) / (1.0*RAND_MAX + 1) * (b - a) + a);
}

bool Client::SendNumber() {
    int num = _random(minRand, state == State::ALIVE ? minRandForAlive : minRandForDead);
    syslog(LOG_INFO, "Sent number %i", num);
    bool rc = conn->Write(reinterpret_cast<void *>(&num), sizeof(num));
    sem_post(semWrite);
    return rc;
}

bool Client::GetState() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += connTimeout;
    if (sem_timedwait(semRead, &ts) == -1)
        return false;
    bool rc = conn->Read(reinterpret_cast<void *>(&state), sizeof(state));
    if (rc) {
        std::string stateStr = "Alive";
        if (state == State::DEAD)
            stateStr = "Dead";
        else if (state == State::FINISH)
            stateStr = "Finish";
        syslog(LOG_INFO, "Got state %s", stateStr.c_str());
    }
    return rc;
}

void Client::Run() {
    syslog(LOG_INFO, "Start client");

    if (kill(hostPid, SIGUSR1) < 0) {
        syslog(LOG_ERR, "Wrong host pid. Terminate.");
        exit(EXIT_FAILURE);
    }


    if (!OpenConnection()) {
        syslog(LOG_ERR, "Could not connect host. Terminate");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Start game");
    do {
        if (!SendNumber()) {
            syslog(LOG_ERR, "Could not send number. Terminate");
            exit(EXIT_FAILURE);
        }

        if (!GetState()) {
            syslog(LOG_ERR, "Could not get state. Terminate");
            exit(EXIT_FAILURE);
        }
    } while(state != State::FINISH);

    syslog(LOG_INFO, "Finish client");
    conn->Close();
}

int main(int argc, char* argv[]) {
    pid_t hostPid;
    if (argc != 2) {
        std::cout << "Wrong number of arguments. Host pid required.\n";
        return EXIT_FAILURE;
    }
    try {
        hostPid = std::atoi(argv[1]);
    }
    catch (...) {
        std::cout << "Wrong type of argument. Host pid must be integer number.\n";
        return EXIT_FAILURE;
    }

    Client client(hostPid);
    client.Run();

    return EXIT_SUCCESS;
}
