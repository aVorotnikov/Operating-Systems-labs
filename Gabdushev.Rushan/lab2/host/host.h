#pragma once
#include <memory>
#include <atomic>
#include <thread>
#include <csignal>
#include "utils/message.h"
#include "utils/safe_queue.h"

class QApplication;
class GUIWindow;
class SharedGameState;
class ClientHandler;

class Host
{
private:
    std::unique_ptr<GUIWindow> window;
    std::shared_ptr<SharedGameState> gameState;
    std::vector<std::shared_ptr<ClientHandler>> clientHandlers;
    std::vector<std::thread> clientHandlerThreads;
    std::shared_ptr<SafeQueue<HostMessage>> fromGUIMessageQueue;
    std::shared_ptr<SafeQueue<Message>> fromClientHandlersMessageQueue;
    static constexpr std::chrono::milliseconds SLEEP_DURATION = std::chrono::milliseconds(100);
    std::atomic_bool needToStop;
    std::thread logicThread;

public:
    static Host &getInstance();
    int run(int argc, char *argv[]);
    ~Host();

private:
    Host();
    Host(Host &w) = delete;
    Host &operator=(const Host &w) = delete;
    void runGUI();
    void initGUI();
    bool initClients(ushort count);
    void runLogic();
    bool createClientHandler(short goatId);
    void runClientHandlers();
    bool fromGUIMessageCheck();
    bool fromClientHandlersMessageCheck();
    void checkRequests();
    void endClients();
    void stop();
    static void signalHandle(int sig, siginfo_t *sigInfo, void *ptr);
};
