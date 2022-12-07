#include <QApplication>
#include "host.h"
#include "wolf_window/gui_window.h"
#include "shared_game_state.h"
#include "client_handler.h"
#include <memory>
#include <chrono>
#include <iostream>
#include <thread>
#include <sys/syslog.h>


Host &Host::getInstance()
{
    static Host instance;
    return instance;
}

void Host::signalHandle(int sig, siginfo_t *sigInfo, void *ptr)
{
    switch (sig)
    {
    case SIGTERM:
        syslog(LOG_INFO, "INFO: host terminate");
        Host::getInstance().stop();
        return;
    case SIGINT:
        syslog(LOG_INFO, "INFO: host terminate");
        Host::getInstance().stop();
        return;
    default:
        syslog(LOG_INFO, "INFO: unknown command");
    }
}

int main(int argc, char *argv[])
{
    return Host::getInstance().run(argc, argv);
}

Host::Host()
{
    needToStop = false;
    gameState = std::make_shared<SharedGameState>();
    fromGUIMessageQueue = std::make_shared<SafeQueue<HostMessage>>();
    fromClientHandlersMessageQueue = std::make_shared<SafeQueue<Message>>();
}
Host::~Host()
{
    if (logicThread.joinable())
    {
        logicThread.join();
    }
}


int Host::run(int argc, char *argv[])
{
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = Host::signalHandle;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGINT, &sig, nullptr);
    if (argc < 2)
    {
        syslog(LOG_ERR, "ERROR: Client count not found");
        std::cout << "ERROR: Expected clients count" << std::endl;
        return EXIT_FAILURE;
    }
    int clientCount;
    try
    {
        clientCount = std::stoi(argv[1]);
    }
    catch (std::exception const &e)
    {
        syslog(LOG_ERR, "ERROR: Client count not be parsed");
        std::cout << "ERROR: Expected clients count" << std::endl;
        return EXIT_FAILURE;
    }
    if (clientCount <= 0)
    {
        syslog(LOG_ERR, "ERROR: Client count must be greater than 0");
        std::cout << "ERROR: Client count must be greater than 0" << std::endl;
        return EXIT_FAILURE;
    }
    if (!initClients(clientCount))
    {
        return EXIT_FAILURE;
    }
    runClientHandlers();
    {
        int argcGUI = 1;
        char *argvGUI[] = {argv[0]};
        auto app = std::make_unique<QApplication>(argcGUI, argvGUI);

        Host::getInstance().initGUI();
        logicThread = std::thread(&Host::runLogic, this);
        Host::getInstance().runGUI();
    }

    logicThread.join();
    return EXIT_SUCCESS;
}

void Host::stop()
{
    needToStop = true;
}

bool Host::initClients(ushort count)
{
    syslog(LOG_INFO, "Host starts clients initialization");
    for (ushort i = 0; i < count; ++i)
    {
        auto goatId = gameState->addGoat();
        if (!createClientHandler(goatId))
        {
            syslog(LOG_INFO, "ERROR: clients initialization failed");
            endClients();
            return false;
        }
    }
    syslog(LOG_INFO, "Host ends clients initialization");
    return true;
}


void Host::runClientHandlers()
{
    for (auto &handler : clientHandlers)
    {
        clientHandlerThreads.push_back(std::thread(&ClientHandler::run, handler.get()));
    }
}

void Host::initGUI()
{
    window = std::make_unique<GUIWindow>(gameState, SafeQueuePusher(fromGUIMessageQueue));
    window->show();
}
void Host::runGUI()
{
    syslog(LOG_INFO, "GUI start");
    qApp->exec();
    syslog(LOG_INFO, "GUI end");
}

void Host::runLogic()
{
    syslog(LOG_INFO, "Host logic loop start");
    while (!needToStop)
    {
        if (!fromGUIMessageCheck())
        {
            break;
        }
        if (!fromClientHandlersMessageCheck())
        {
            break;
        }
        std::this_thread::sleep_for(SLEEP_DURATION);
    }
    window->pushMessage(GUIMessage(GUIMessage::GMT_END));
    endClients();
    syslog(LOG_INFO, "Host logic loop end");
}

void Host::endClients()
{
    for (auto &handler : clientHandlers)
    {
        handler->pushMessage(Message::MT_END);
    }
    for (auto &thread : clientHandlerThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}
bool Host::createClientHandler(short goatId)
{
    auto handler = std::make_shared<ClientHandler>(goatId, gameState, SafeQueuePusher(fromClientHandlersMessageQueue));
    if (!handler->init())
    {
        syslog(LOG_ERR, "ERROR: failed to init ClientHandler");
        return false;
    }
    clientHandlers.push_back(handler);
    return true;
}

bool Host::fromGUIMessageCheck()
{
    while (fromGUIMessageQueue->size() != 0)
    {
        HostMessage message = fromGUIMessageQueue->front();
        fromGUIMessageQueue->pop();
        switch (message.messageType)
        {
        case HostMessage::HMT_END:
            stop();
            return false;
        case HostMessage::HMT_START_GAME:
        {
            auto goatTable = gameState->getGoatTable();
            for (auto &info : goatTable)
            {
                gameState->updateGoatInfo(GoatInfo(info.id));
            }
            window->pushMessage(GUIMessage("New game started"));
            window->pushMessage(GUIMessage(GUIMessage::GMT_UPDATE_TABLE));
            window->pushMessage(GUIMessage(GUIMessage::GMT_NEW_ROUND));
            window->pushMessage(GUIMessage("Next round"));
            break;
        }
        case HostMessage::HMT_GUI_INPUT_END:
        {
            window->pushMessage(GUIMessage("Wolf number is " + std::to_string(gameState->wolfNumber)));
            auto goatTable = gameState->getGoatTable();
            for (auto &info : goatTable)
            {
                info.lastEvent = GOAT_EVENT::THROW_REQUESTED;
                gameState->updateGoatInfo(info);
            }
            window->pushMessage(GUIMessage(GUIMessage::GMT_UPDATE_TABLE));
            for (auto &handler : clientHandlers)
            {
                handler->pushMessage(Message::MT_THROW_REQUEST);
            }
            window->pushMessage(GUIMessage("Waiting for goat numbers"));
            break;
        }
        default:
            syslog(LOG_ERR, "ERROR: Host got unknown message type from GUI");
            stop();
            return false;
        }
    }
    return true;
}

bool Host::fromClientHandlersMessageCheck()
{
    while (fromClientHandlersMessageQueue->size() != 0)
    {
        Message message = fromClientHandlersMessageQueue->front();
        fromClientHandlersMessageQueue->pop();
        switch (message.messageType)
        {
        case Message::MT_END:
            window->pushMessage(GUIMessage("Goat " + std::to_string(message.goatInfo.id) + " disconected"));
            gameState->removeGoatInfo(message.goatInfo.id);
            if (gameState->size() == 0)
            {
                window->pushMessage(GUIMessage("All goats disconected"));
                stop();
                return false;
            }
            checkRequests();
            break;
        case Message::MT_THROW_RESPONSE:
            gameState->updateGoatInfo(message.goatInfo);
            window->pushMessage(GUIMessage(GUIMessage::GMT_UPDATE_TABLE));
            window->pushMessage(GUIMessage("Goat " + std::to_string(message.goatInfo.id) + " throws number " + std::to_string(message.goatInfo.thrownNumber)));
            checkRequests();
            break;
        default:
            syslog(LOG_ERR, "ERROR: Host got unknown message type from ClientHandler");
            stop();
            return false;
        }
    }
    return true;
}

void Host::checkRequests()
{
    auto goatTable = gameState->getGoatTable();
    if (std::all_of(goatTable.cbegin(), goatTable.cend(), [](GoatInfo a)
                    { return a.lastEvent == THROW_RECEIVED; }))
    {
        float wolfNumber = static_cast<float>(gameState->wolfNumber);
        int alivePrev = 0;
        int aliveNow = 0;
        int died = 0;
        int revived = 0;
        for (auto &info : goatTable)
        {
            if (info.state == GOAT_STATE::ALIVE)
            {
                ++alivePrev;
                if (abs(static_cast<float>(info.thrownNumber) - wolfNumber) <= 70.f / goatTable.size())
                {
                    ++aliveNow;
                    info.lastEvent = GOAT_EVENT::HIDEN;
                }
                else
                {
                    ++died;
                    info.state = GOAT_STATE::DEAD;
                    info.lastEvent = GOAT_EVENT::DIED;
                }
            }
            else
            {
                if (abs(static_cast<float>(info.thrownNumber) - wolfNumber) <= 20.f / goatTable.size())
                {
                    ++aliveNow;
                    ++revived;
                    info.state = GOAT_STATE::ALIVE;
                    info.lastEvent = GOAT_EVENT::REVIVED;
                }
                else
                {
                    info.lastEvent = GOAT_EVENT::DIED;
                }
            }
            gameState->updateGoatInfo(info);
        }
        window->pushMessage(GUIMessage("Round result: " + std::to_string(aliveNow) + " alive, "  + std::to_string(died) + " died, "  + std::to_string(revived) + " revived" ));
        window->pushMessage(GUIMessage(GUIMessage::GMT_UPDATE_TABLE));
        for (auto &handler : clientHandlers)
        {
            handler->pushMessage(Message::MT_ROUND_RESULT);
        }
        if (alivePrev == 0 && aliveNow == 0)
        {
            window->pushMessage(GUIMessage("Game over"));
            window->pushMessage(GUIMessage(GUIMessage::GMT_GAME_OVER));
        }
        else
        {
            window->pushMessage(GUIMessage("Next round"));
            window->pushMessage(GUIMessage(GUIMessage::GMT_NEW_ROUND));
        }
    }
}
