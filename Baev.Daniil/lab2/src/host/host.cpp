#include <sys/syslog.h>
#include <thread>
#include <csignal>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include <fcntl.h>

#include "../gui/gui.h"
#include "host.h"
#include "../client/client.h"

std::atomic<bool> Host::_isTerminated = false;

int main(int argc, char *argv[]) {
    openlog("lab2_host", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);

    Host host;
    int status = EXIT_SUCCESS;
    pid_t hostPid = getpid();
    printf("host pid = %i\n", getpid());

    host.init(3);
    if (getpid() == hostPid){
        status = host.run();
    }

    syslog(LOG_INFO, "End process %d\n", getpid());
    closelog();
    return status;
}

Host::Host(void) {
    struct sigaction sig{};
    std::memset(&sig, 0, sizeof(sig));
    sig.sa_sigaction = signalHandler;
    sig.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &sig, nullptr);
}

void Host::init(int clientNum){
    bool isHost = true;
    _gst = std::make_shared<GameState>();
    _gst->deadNumber = 0;
    _gst->aliveNumber = 0;

    for (int i = 0; i < clientNum; i++){
        try {
            auto handler = std::make_shared<ClientHandler>();
            isHost = handler->forkClient(i, _gst);
            if (isHost){
                std::thread clientHandlerThread(&ClientHandler::run, handler);
                clientHandlerThread.detach();
                _clientHandler.push(handler);
                syslog(LOG_INFO, "Open heandler in thread");
            }
            else{
                break;
            }
        }
        catch (std::exception &e) {
            syslog(LOG_ERR, "Error: %s", e.what());
        }
    }
}

int Host::run(void){
    syslog(LOG_INFO, "Host started");
    _isTerminated = false;

    try {
        std::thread connectionThread(&Host::wolfLogic, this);

        int argc = 1;
        char* args[] = { (char*)"WolfAndGoat" };
        QApplication app(argc, args);

        syslog(LOG_INFO, "Create_window");
        GUI window(_gst);

        _gst->time = 3;
        _timer = std::make_unique<QTimer>(this);
        QObject::connect(_timer.get(), SIGNAL(timeout()), this, SLOT(updateTimer()));
        _timer->start(1000);

        QObject::connect(&window, SIGNAL(wolfNumberSend(int)), this, SLOT(wolfNumberEnter(int)));
        QObject::connect(this, SIGNAL(gameover()), &window, SLOT(gameover()));
        window.SetTitle(args[0]);
        window.show();
        app.exec();

        terminate();
        connectionThread.join();
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "Error: %s", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void Host::wolfLogic(void) {
    printf("host pid = %i\n", getpid());
    std::srand(std::time(0));

    bool allDied = false;

    while (!_isTerminated.load()){
        if (_gst->aliveNumber.load() == 0){
            if (allDied == true){
                syslog(LOG_INFO, "Gameover");
                emit gameover();
                std::this_thread::sleep_for(std::chrono::seconds(3));
                terminate();
            }
            else{
                allDied = true;
            }
        }
        else{
            allDied = false;
        }
        if (_wolf._randomNumber.load()){
            _wolf._number = 1 + std::rand() % _wolf._MaxNum;
        }
        _gst->wolfNumber = _wolf._number.load();
        for (size_t i = 0; i < _clientHandler.len(); ++i){
            _clientHandler[i]->sendRequest();
        }
        _wolf._randomNumber = true;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        _gst->time = 3;
    }
    for (size_t i = 0; i < _clientHandler.len(); ++i){
        syslog(LOG_INFO, "Close %d", int(i));
        _clientHandler[i]->close();
    }
}

void Host::signalHandler(int signum, siginfo_t *info, void *ptr) {
    switch (signum) {
        case SIGTERM:
            Host::terminate();
            break;
    }
}

void Host::terminate() noexcept {
    if (!_isTerminated.load()) {
        _isTerminated = true;
        syslog(LOG_INFO, "Terminating host");
    }
}

void Host::wolfNumberEnter(int number){
    _wolf._randomNumber = false;
    _wolf._number = number;
}

void Host::updateTimer(){
    _gst->time--;
}