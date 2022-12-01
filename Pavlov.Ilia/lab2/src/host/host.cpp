#include "gui/mainwindow.h"
#include "host.h"
#include "../connections/conn.h"

#include <QApplication>
#include <syslog.h>
#include <sys/types.h>
#include <fcntl.h>

Host Host::instance;
Host* Host::GetInstance() {
    return &instance;
}

void SignalHandler(int signum, siginfo_t *si, void *data) {
  switch (signum) {
  case SIGTERM:
      Host::GetInstance()->Terminate(EXIT_SUCCESS);
      break;
  case SIGUSR1:
      Host::GetInstance()->ConnectClient(si->si_pid);
      break;
  default:
    break;
  }
}

Host::Host() : conn(Conn::GetConn(pid, Conn::Type::HOST)){
    struct sigaction sig{};
    memset(&sig, 0, sizeof(sig));
    sig.sa_sigaction = SignalHandler;
    sig.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGUSR1, &sig, nullptr);
    openlog("host", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);

    std::string semReadPath = "/sem1" + std::to_string(pid);
    semRead = sem_open(semReadPath.c_str(), O_CREAT | O_EXCL, 0777, 0);
    if (semRead == SEM_FAILED) {
        syslog(LOG_ERR, "Could not open semaphore");
        Terminate(EXIT_FAILURE);
    }

    std::string semWritePath = "/sem2" + std::to_string(pid);
    semWrite = sem_open(semWritePath.c_str(), O_CREAT | O_EXCL, 0777, 0);
    if (semWrite == SEM_FAILED) {
        syslog(LOG_ERR, "Could not open semaphore");
        Terminate(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Semaphores opened");
    srand(time(nullptr));
}

void Host::Terminate(int status) {
    syslog(LOG_INFO, "Terminate host");
    exit(status);
}

void Host::SetWindow(MainWindow* window) {
    mw = window;
    mw->SetPid(pid);
    mw->WriteLog("Запустите клиента с указанным pid хоста");
}

pid_t Host::GetPid() {
    return pid;
}

void Host::ConnectClient(pid_t clientPid) {
    this->clientPid = clientPid;
    if (!conn->Open()) {
        syslog(LOG_ERR, "Could not open connection");
        mw->WriteLog("Could not open connection");
        Terminate(EXIT_FAILURE);
    }
    mw->WriteLog("Клиент подключен");
    StartGame();
}

void Host::StopClient() {
    if (clientPid != -1) {
        syslog(LOG_INFO, "Stop client");
        kill(clientPid, SIGTERM);
    }
}

void Host::StartGame() {
    bool dead_first_round = false;

    while (true) {
        if (!StartRound()) {
            mw->WriteLog("Потеряно соединение. Игра окончена");
            Terminate(EXIT_FAILURE);
        }
        if (!dead_first_round && clientState == State::DEAD)
            dead_first_round = true;
        else if (dead_first_round && clientState == State::DEAD)
            break;
        else
            dead_first_round = false;
    }
    mw->WriteLog("Game over");
    StopClient();
}

bool Host::GetClientNum(int* num) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout;
    if (sem_timedwait(semRead, &ts) == -1)
        return false;
    return conn->Read(num, sizeof(num));
}

bool Host::SendState(State state) {
    bool rc = conn->Write(&state, sizeof(state));
    sem_post(semWrite);
    return rc;
}

bool Host::StartRound() {
    static int roundNum = 1;
    mw->WriteLog("Раунд № " + std::to_string(roundNum++));
    int hostNum = mw->GetNumber();
    int clientNum;
    if (!GetClientNum(&clientNum)) {
        return false;
    }

    std::string msg = clientState == State::ALIVE ? "жив" : "мёртв";
    if (clientState == State::ALIVE && abs(clientNum - hostNum) > diffForAlive) {
        clientState = State::DEAD;
        msg = "убит";
    }
    else if (clientState == State::DEAD && abs(clientNum - hostNum) <= diffForDead) {
        clientState = State::ALIVE;
        msg = "воскрес";
    }

    if (!SendState(clientState)) {
        return false;
    }

    mw->WriteLog("Число волка: " + std::to_string(hostNum));
    mw->WriteLog("Число козлёнка: " + std::to_string(clientNum));
    mw->WriteLog("Козлёнок " + msg);
    mw->WriteLog("----------------");

    return true;
}

Host::~Host() {
    conn->Close();
    sem_close(semRead);
    sem_close(semWrite);
    closelog();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    Host::GetInstance()->SetWindow(&w);
    w.show();
    return a.exec();
}
