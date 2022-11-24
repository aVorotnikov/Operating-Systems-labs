#include <sys/syslog.h>
#include <csignal>
#include <ctime>
#include <thread>
#include <chrono>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

#include "client.h"

int main(int argc, char *argv[]) {
    openlog("lab2_client", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);

    if (argc != 2) {
        syslog(LOG_ERR, "Expected host pid as the command argument");
        closelog();
        return EXIT_FAILURE;
    }

    int pid;
    try {
        pid = std::stoi(argv[1]);
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "Pid reading error: %s", e.what());
        closelog();
        return EXIT_FAILURE;
    }

    int status = Client::GetInstance().Run(pid);

    closelog();
    return status;
}

void Client::SignalHandler(int sig) {
    switch (sig) {
        case SIGTERM:
            Client::GetInstance().Terminate();
        break;
    }
}

Client::Client(void) {
    syslog(LOG_INFO, "Set signal handler");
    std::signal(SIGTERM, SignalHandler);
}

bool Client::OpenConnection(void) {
    syslog(LOG_INFO, "Creating connection for %d", int(_hostPid));
    try {
        _conn = Connection::Create(getpid(), false);
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "Connection creation error");
        return false;
    }
    catch (const char *e) {
        syslog(LOG_ERR, "Connection creation error");
        return false;
    }

    std::string semNameRead = "/client_" + std::to_string(getpid());
    std::string semNameWrite = "/host_" + std::to_string(getpid());
    _semRead = sem_open(semNameRead.c_str(), 0);
    if (_semRead == SEM_FAILED) {
        syslog(LOG_ERR, "Semaphore creation error");
        return false;
    }
    _semWrite = sem_open(semNameWrite.c_str(), 0);
    if (_semWrite == SEM_FAILED) {
        syslog(LOG_ERR, "Semaphore creation error");
        sem_close(_semRead);
        return false;
    }

    syslog(LOG_INFO, "Connection opened");
    return true;
}

bool Client::GetState(void) {
    timespec t;

    clock_gettime(CLOCK_REALTIME, &t);

    t.tv_sec += _timeInter;

    int s = sem_timedwait(_semRead, &t);
    if (s == -1) {
        syslog(LOG_ERR, "Read semaphore timeout");
        return false;
    }

    try {
        _conn->Read(&_isAlive, sizeof(bool));
        return true;
    }
    catch (const char *e) {
        syslog(LOG_ERR, "Connection reading error");
        return false;
    }
}

bool Client::SendNum(void) {
    std::srand(std::time(0));
    int k = _isAlive ? _maxNum : _maxNum / _divider;
    int num = 1 + rand() % k;

    try {
        _conn->Write(&num, sizeof(int));
        sem_post(_semWrite);
        return true;
    }
    catch (const char *e) {
        syslog(LOG_ERR, "Connection writing error");
        return false;
    }
}

void Client::CloseConnection(void) {
    sem_close(_semRead);
    sem_close(_semWrite);
}

int Client::Run(pid_t hostPid) noexcept {
    syslog(LOG_INFO, "Client started");

    _hostPid = hostPid;
    _isTerminated = false;

    if (kill(hostPid, SIGUSR1) != 0) {
        syslog(LOG_ERR, "Host %d not present", hostPid);
        return EXIT_FAILURE;
    }
    syslog(LOG_ERR, "Signal sent");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (!OpenConnection())
        return EXIT_FAILURE;

    while (!_isTerminated.load()) {
        if (!GetState()) {
            CloseConnection();
            return EXIT_FAILURE;
        }

        if (!SendNum()) {
            CloseConnection();
            return EXIT_FAILURE;
        }
    }
    CloseConnection();
    return EXIT_SUCCESS;
}

void Client::Terminate(void) noexcept {
    if (!_isTerminated.load()) {
        _isTerminated = true;
        syslog(LOG_INFO, "Terminating client");
    }
}