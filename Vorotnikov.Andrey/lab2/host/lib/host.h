#pragma once

#include "../gui/gui.h"
#include "../../connections/conn.h"

#include <semaphore.h>
#include <bits/types/siginfo_t.h>

#include <optional>
#include <condition_variable>
#include <mutex>
#include <memory>

class Host
{
public:
    friend void SignalHandler(int signum, siginfo_t *info, void *ptr);

    static Host& GetRef();

    using ConnectionCreator = std::function<std::shared_ptr<Connection> (pid_t pid)>;
    void SetConnection(std::shared_ptr<Connection> connection);
    bool Run();
    void Terminate();

private:
    static constexpr char windowName_[] = "Wolf and goats";
    static constexpr char deadGhoatMessage_[] = "dead";
    static constexpr char aliveGhoatMessage_[] = "alive";

    static constexpr int timeoutOnSem_ = 10;
    static constexpr int timeForInput_ = 3;

    static Host instance_;
    Host();

    Host(const Host&) = delete;
    Host& operator=(const Host&) = delete;

    void Work();

    bool OpenConnection();
    bool GetNum(int& number);
    bool SendState(bool isAlive);

    ConnectionCreator connectionCreator_;

    std::optional<GameState> gameState_;
    std::mutex gameStateMtx_;
    std::optional<int> input_;
    std::mutex inputMtx_;
    std::condition_variable numberGetted_;
    std::mutex numberGettedMtx_;

    pid_t clientPid_;
    bool isTerminated_;
    int timeToRender_;

    std::shared_ptr<Connection> connection_;
    sem_t *semOnRead_;
    sem_t *semOnWrite_;
};
