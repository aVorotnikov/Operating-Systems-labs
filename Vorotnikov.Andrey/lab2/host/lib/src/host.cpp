#include "../host.h"
#include "../../../game_rules/game_rules.h"
#include "../../../utils/raw.h"

#include <sys/syslog.h>
#include <thread>
#include <csignal>
#include <chrono>
#include <ctime>
#include <semaphore.h>
#include <cstring>
#include <fcntl.h>

extern Host Host::instance_;

Host& Host::GetRef()
{
    return instance_;
}

void SignalHandler(int signum, siginfo_t *info, void *ptr)
{
    switch (signum)
    {
    case SIGUSR1:
        syslog(LOG_INFO, "Client %d request connection to host", info->si_pid);
        if (-1 == Host::instance_.clientPid_)
            Host::instance_.clientPid_ = info->si_pid;
        else
            syslog(LOG_INFO, "Host has already client %i", Host::instance_.clientPid_);
        break;
    case SIGTERM:
        Host::instance_.Terminate();
        break;
    }
}

Host::Host() :
    clientPid_(-1), isTerminated_(false), timeToRender_(0),
    connection_(nullptr), semOnRead_(nullptr), semOnWrite_(nullptr)
{
    struct sigaction sig{};
    std::memset(&sig, 0, sizeof(sig));
    sig.sa_sigaction = SignalHandler;
    sig.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGUSR1, &sig, nullptr);
}

void Host::SetConnection(std::shared_ptr<Connection> connection)
{
    connection_ = connection;
}

bool Host::OpenConnection()
{
    constexpr char semaphoreOnReadNameTemplate[] = "/host_";
    constexpr char semaphoreOnWriteNameTemplate[] = "/client_";
    constexpr int semaphoreFlags = 0777;

    syslog(LOG_INFO, "Creating connection");
    if (!connection_)
        return false;

    std::string semaphoreOnReadName = semaphoreOnReadNameTemplate + std::to_string(connection_->hostPid_);
    std::string semaphoreOnWriteName = semaphoreOnWriteNameTemplate + std::to_string(connection_->hostPid_);
    semOnRead_ = sem_open(semaphoreOnReadName.c_str(), O_CREAT | O_EXCL, semaphoreFlags, 0);
    if (semOnRead_ == SEM_FAILED)
    {
        syslog(LOG_ERR, "Semaphore creation error");
        clientPid_ = -1;
        return false;
    }
    semOnWrite_ = sem_open(semaphoreOnWriteName.c_str(), O_CREAT | O_EXCL, semaphoreFlags, 0);
    if (semOnWrite_ == SEM_FAILED)
    {
        syslog(LOG_ERR, "Semaphore creation error");
        sem_close(semOnRead_);
        clientPid_ = -1;
        return false;
    }

    syslog(LOG_INFO, "Connection opened");
    return true;
}

bool Host::GetNum(int& number)
{
    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += timeoutOnSem_;

    int waiting = sem_timedwait(semOnRead_, &t);
    if (-1 == waiting)
    {
        syslog(LOG_ERR, "Read semaphore timeout");
        return false;
    }

    if (utils::GetRaw(connection_, number))
        return true;
    syslog(LOG_ERR, "Connection reading error");
    return false;
}

bool Host::SendState(bool isAlive)
{
    if (utils::SendRaw(connection_, isAlive))
    {
        sem_post(semOnWrite_);
        return true;
    }
    else
    {
        syslog(LOG_ERR, "Connection writing error");
        return false;
    }
}

bool Host::Run()
{
    syslog(LOG_INFO, "Host started");
    isTerminated_ = false;
    clientPid_ = -1;

    try
    {
        std::thread connectionThread(&Host::Work, this);

        int argc = 1;
        char* args[] = {const_cast<char*>(windowName_)};
        QApplication app(argc, args);
        Gui window(
            [](int number)
            {
                std::lock_guard<std::mutex> lock(instance_.inputMtx_);
                instance_.input_ = number;
                instance_.numberGetted_.notify_one();
            },
            [](GameState& gameState) -> bool
            {
                std::lock_guard<std::mutex> lock(instance_.gameStateMtx_);
                auto& gameState_ = instance_.gameState_;
                if (!gameState_.has_value())
                    return false;
                gameState = gameState_.value();
                gameState_.reset();
                return true;
            },
            []() {return instance_.timeToRender_;},
            []() {return instance_.isTerminated_;},
            []() {return -1 != instance_.clientPid_;},
            []()
            {
                std::lock_guard<std::mutex> lock(instance_.inputMtx_);
                return !instance_.input_.has_value();
            });

        window.SetTitle(windowName_);
        window.show();
        app.exec();

        Terminate();
        connectionThread.join();
    }
    catch (std::exception &e)
    {
        syslog(LOG_ERR, "Error: %s", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void Host::Work()
{
    printf("host pid = %i\n", connection_->hostPid_);
    auto checkPoint = std::chrono::high_resolution_clock::now();
    std::size_t lossesNumber = 0;
    GameState gameState{0, 0, ""};
    bool goatIsAlive = true;

    if (!OpenConnection())
        return;

    while (!isTerminated_)
    {
        if (clientPid_ == -1)
        {
            auto timePassed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - checkPoint).count();

            if (timePassed >= 60)
                Terminate();
            continue;
        }

        while (!isTerminated_)
        {
            if (lossesNumber >= game_rules::roundsNumberToWin)
            {
                syslog(LOG_INFO, "Killing client after %lu losses", game_rules::roundsNumberToWin);
                kill(clientPid_, SIGTERM);
                clientPid_ = -1;
                break;
            }

            if (!SendState(goatIsAlive))
                break;

            if (!GetNum(gameState.goatNumber))
                break;

            timeToRender_ = timeForInput_;
            while (timeToRender_ > 0 && !input_.has_value())
            {
                std::unique_lock lock(numberGettedMtx_);
                numberGetted_.wait_for(lock, std::chrono::seconds(1),
                    [host = this]() {return host->input_.has_value();});
                --timeToRender_;
            }

            std::unique_lock<std::mutex> lock(inputMtx_);
            if (input_.has_value())
            {
                gameState.wolfNumber = std::clamp(input_.value(), game_rules::wolfRange.min, game_rules::wolfRange.max);
            }
            else
                gameState.wolfNumber = game_rules::GetRandomForWolf();
            input_.reset();
            lock.unlock();

            if (goatIsAlive)
            {
                if (game_rules::CheckStateForAliveGoat(gameState.wolfNumber, gameState.goatNumber, 1))
                    lossesNumber = 0;
                else
                {
                    ++lossesNumber;
                    goatIsAlive = false;
                }
            }
            else
            {
                if (game_rules::CheckStateForDeadGoat(gameState.wolfNumber, gameState.goatNumber, 1)) {
                    goatIsAlive = true;
                    lossesNumber = 0;
                }
                else
                    ++lossesNumber;
            }

            if (goatIsAlive)
                gameState.goatState = aliveGhoatMessage_;
            else
                gameState.goatState = deadGhoatMessage_;

            std::unique_lock<std::mutex> lock1(gameStateMtx_);
            gameState_ = gameState;
            lock1.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (clientPid_ != -1)
            kill(clientPid_, SIGTERM);
        lossesNumber = 0;
        goatIsAlive = true;
        checkPoint = std::chrono::high_resolution_clock::now();
    }
    sem_close(semOnRead_);
    sem_close(semOnWrite_);

    if (clientPid_ != -1)
        kill(clientPid_, SIGTERM);
}

void Host::Terminate()
{
    if (!isTerminated_)
    {
        isTerminated_ = true;
        syslog(LOG_INFO, "Terminating host");
    }
}
