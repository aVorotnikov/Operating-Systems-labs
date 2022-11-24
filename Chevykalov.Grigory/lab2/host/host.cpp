#include <sys/syslog.h>
#include <thread>
#include <csignal>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include <fcntl.h>

#include "host.h"

int main(int argc, char *argv[]) {
    openlog("lab2_host", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);

    int status = Host::GetInstance().Run();

    closelog();
    return status;
}

void Host::SignalHandler(int signum, siginfo_t *info, void *ptr) {
    switch (signum) {
        case SIGUSR1:
            syslog(LOG_INFO, "Client %d request connection to host", info->si_pid);
            if (Host::GetInstance()._clientPid.load() == -1)
                Host::GetInstance()._clientPid = info->si_pid;
            else
                syslog(LOG_INFO, "Host has already client %d", Host::GetInstance()._clientPid.load());
            break;
        case SIGTERM:
            Host::GetInstance().Terminate();
            break;
    }
}

Host::Host(void) {
    struct sigaction sig{};
    std::memset(&sig, 0, sizeof(sig));
    sig.sa_sigaction = SignalHandler;
    sig.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGUSR1, &sig, nullptr);
}

bool Host::OpenConnection(void) {
    syslog(LOG_INFO, "Creating connection for %d", int(_clientPid.load()));
    try {
        _conn = Connection::Create(_clientPid.load(), true);
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "Connection creation error");
        _clientPid = -1;
        return false;
    }
    catch (const char *e) {
        syslog(LOG_ERR, "Connection creation error");
        _clientPid = -1;
        return false;
    }

    std::string semNameRead = "/host_" + std::to_string(_clientPid.load());
    std::string semNameWrite = "/client_" + std::to_string(_clientPid.load());
    _semRead = sem_open(semNameRead.c_str(), O_CREAT | O_EXCL, 0777, 0);
    if (_semRead == SEM_FAILED) {
        syslog(LOG_ERR, "Semaphore creation error");
        _clientPid = -1;
        return false;
    }
    _semWrite = sem_open(semNameWrite.c_str(), O_CREAT | O_EXCL, 0777, 0);
    if (_semWrite == SEM_FAILED) {
        syslog(LOG_ERR, "Semaphore creation error");
        sem_close(_semRead);
        _clientPid = -1;
        return false;
    }

    syslog(LOG_INFO, "Connection opened");
    return true;
}

bool Host::GetNum(int *num) {
    timespec t;

    clock_gettime(CLOCK_REALTIME, &t);

    t.tv_sec += _timeInter;

    int s = sem_timedwait(_semRead, &t);
    if (s == -1) {
        syslog(LOG_ERR, "Read semaphore timeout");
        return false;
    }

    try {
        _conn->Read(num, sizeof(int));
        return true;
    }
    catch (const char *e) {
        syslog(LOG_ERR, "Connection reading error");
        return false;
    }
}

bool Host::SendState(bool st) {
    try {
        _conn->Write(&st, sizeof(bool));
        sem_post(_semWrite);
        return true;
    }
    catch (const char *e) {
        syslog(LOG_ERR, "Connection writing error");
        return false;
    }
}

void Host::CloseConnection(void) {
    sem_close(_semRead);
    sem_close(_semWrite);
}

int Host::Run(void) noexcept {
    syslog(LOG_INFO, "Host started");
    _isTerminated = false;
    _clientPid = -1;

    try {
        std::thread connectionThread(&Host::Work, this);

        int argc = 1;
        char* args[] = { (char*)"WolfAndGoat" };
        QApplication app(argc, args);
        GUI window((sendClb)GUISend, (getClb)GUIGet, (tClb)GUITimer, (isSmthClb)GUIIsTerminated,
            (isSmthClb)GUIIsConnected, (isSmthClb)GUINeedToSend);

        window.SetTitle(args[0]);
        window.show();
        app.exec();

        Terminate();
        connectionThread.join();
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "Error: %s", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void Host::Work(void) {
    printf("host pid = %i\n", getpid());
    auto checkPoint = std::chrono::high_resolution_clock::now();
    int numLosses = 0;
    GState gst{0, 0, ""};
    bool goatIsAlive = true;
    std::srand(std::time(0));

    while (!_isTerminated.load()) {
        if (_clientPid.load() == -1) {
            auto timePassed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - checkPoint).count();

            if (timePassed >= 60)
                Terminate();
            continue;
        }

        if (!OpenConnection())
            continue;

        while (!_isTerminated.load()) {
            if (numLosses >= _lMax) {
                syslog(LOG_INFO, "Killing client after 2 losses");
                kill(_clientPid.load(), SIGTERM);
                _clientPid = -1;
                break;
            }

            if (!SendState(goatIsAlive))
                break;

            if (!GetNum(&gst.goatN))
                break;

            _time = _tMax;
            while (_time.load() > 0) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                --_time;
            }

            int num = 0;
            if (_input.Get(&num))
                gst.wolfN = num;
            else
                gst.wolfN = 1 + std::rand() % _maxNum;

            if (goatIsAlive) {
                if (abs(gst.goatN - gst.wolfN) > _aliveInter) {
                    ++numLosses;
                    goatIsAlive = false;
                }
                else
                    numLosses = 0;
            }
            else
                if (abs(gst.goatN - gst.wolfN) <= _deadInter) {
                    goatIsAlive = true;
                    numLosses = 0;
                }
                else
                    ++numLosses;

            if (goatIsAlive)
                gst.goatSt = ALIVE;
            else
                gst.goatSt = DEAD;

            _output.Push(gst);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (_clientPid.load() != -1)
            kill(_clientPid.load(), SIGTERM);
        CloseConnection();
        numLosses = 0;
        goatIsAlive = true;
        checkPoint = std::chrono::high_resolution_clock::now();
    }

    if (_clientPid.load() != -1)
        kill(_clientPid.load(), SIGTERM);
}

void Host::Terminate() noexcept {
    if (!_isTerminated.load()) {
        _isTerminated = true;
        syslog(LOG_INFO, "Terminating host");
    }
}