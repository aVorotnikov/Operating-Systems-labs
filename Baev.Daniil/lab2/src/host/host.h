#pragma once 

#include <sys/types.h>
#include <atomic>
#include <memory>
#include <csignal>

#include <QObject>
#include <QTimer>

#include "../utils/threadsafe_vector.hpp"
#include "../gui/model.hpp"
#include "../connection/connection.h"
#include "clientHandler.h"

constexpr int const clientNum = 5;

class Host: public QObject {
Q_OBJECT
public:
    static std::atomic<bool> _isTerminated;
    static void signalHandler(int signum, siginfo_t *info, void *ptr);
    static void terminate(void) noexcept;

    Host();
    Host(const Host&) = delete;

    Host& operator=(const Host&) = delete;

    void init(int clientNum);
    int run(void);

    ~Host() = default;
signals:
    void gameover(void);
private slots:
    void wolfNumberEnter(int number);
    void updateTimer();
private:
    struct Wolf {
        const int _MaxNum = 100;
        std::atomic<bool> _randomNumber;
        std::atomic<int> _number;
    }; 
    Wolf _wolf;
    std::unique_ptr<QTimer> _timer;
    std::shared_ptr<GameState> _gst;
    Vec<std::shared_ptr<ClientHandler>> _clientHandler = {};
    void wolfLogic(void);
};
