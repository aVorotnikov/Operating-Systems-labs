#include <sys/syslog.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include <csignal>
#include <chrono>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include "client.h"
#include "../connections/connection.h"

Client Client::m_clientInstance;

int main(int argc, char*argv[])
{
    openlog("lab2_client", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);
    /*
    if (argc != 2)
    {
        syslog(LOG_ERR, "expected host pid as the only command argument");
        closelog();
        return 1;
    }

    int pid;
    try
    {
        pid = std::stoi(argv[1]);
    }
    catch (std::exception &e)
    {
        syslog(LOG_ERR, "An error while pid reading %s", e.what());
        closelog();
        return 1;
    }
    */
    int pid = 3728;
    try
    {
        Client::GetInstance().Run(pid);
    }
    catch (std::exception &e)
    {
        syslog(LOG_ERR, "An error %s", e.what());
    }

    closelog();
    return 0;
}


void Client::SignalHandler(int signum, siginfo_t* info, void *ptr)
{
    switch (signum)
    {
        case SIGUSR1:
        {
            syslog(LOG_INFO, "Got signal that host are ready");
            Client::GetInstance().m_isHostReady = true;
            break;
        }
        case SIGTERM:
        {
            Client::GetInstance().Stop();
            break;
        }
    }
}

Client::Client(void)
{
    // subscribe on signal actions
    struct sigaction sig{};
    memset(&sig, 0, sizeof(sig));
    sig.sa_sigaction = SignalHandler;
    sig.sa_flags = SA_SIGINFO;
    // for termination
    sigaction(SIGTERM, &sig, nullptr);
    // for connecting User
    sigaction(SIGUSR1, &sig, nullptr);
}

void Client::Run(pid_t hostPid)
{
    syslog(LOG_INFO, "Client started");

    m_hostPid = hostPid;
    m_isRunning = true;

    // send signal to Host because we need to make handshake
    if (kill(hostPid, SIGUSR1) != 0)
    {
        // if no such process -> just exit
        m_isRunning = false;
        syslog(LOG_ERR, "Host %d not present", hostPid);
        return;
    }
    syslog(LOG_ERR, "Signal sent");

    // run thread with connections
    std::thread t(&Client::ConnectionWork, this);

    while (m_isRunning.load())
    {
        if (m_isHostReady.load())
        {
            // simple work with messages
            Message msg = {0};
            scanf("%s", msg.m_message);

            m_outputMessagesMutex.lock();
            m_outputMessages.push(msg);
            m_outputMessagesMutex.unlock();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    t.join();
}

void Client::Stop()
{
    m_isRunning = false;
    syslog(LOG_INFO, "Start terminating client");
}

Client::~Client(void)
{
}

void Client::ConnectionWork(void)
{
    try
    {
        auto clock = std::chrono::high_resolution_clock::now();

        while (!m_isHostReady.load())
        {
            double second = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - clock).count();

            if (second >= 5)
            {
                m_isHostReady = false;
                m_hostPid = false;
                syslog(LOG_ERR, "Host cannot prepare for 5 seconds. Exiting");
                Stop();
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }

        syslog(LOG_INFO, "start creating connection for %d", int(m_hostPid));
        Connection *currentConnection = Connection::CreateConnection(getpid(), false);
        if (!currentConnection)
        {
            syslog(LOG_ERR, "Connection creation error");
            return;
        }

        // open semaphores
        sem_t *semaphoreRead, *semaphoreWrite;
        std::string semNameRead = "/client_" + std::to_string(getpid());
        std::string semNameWrite = "/host_" + std::to_string(getpid());
        semaphoreRead = sem_open(semNameRead.c_str(), 0);
        if (semaphoreRead == SEM_FAILED)
        {
            syslog(LOG_ERR, "Semaphore creation error");
            delete currentConnection;
            return;
        }
        syslog(LOG_INFO, "created semaphore %s", semNameRead.c_str());
        semaphoreWrite = sem_open(semNameWrite.c_str(), 0);
        if (semaphoreWrite == SEM_FAILED)
        {
            syslog(LOG_ERR, "Semaphore creation error");
            sem_close(semaphoreRead);
            delete currentConnection;
            return;
        }
        syslog(LOG_INFO, "created semaphore %s", semNameWrite.c_str());
        currentConnection->Open(m_hostPid, false);
        syslog(LOG_INFO, "opened connection");

        // while m_isRunning
        while (m_isRunning.load())
        {
            // Send all messages
            {
                m_outputMessagesMutex.lock();
                uint32_t amount = m_outputMessages.size();
                if (amount > 0)
                    syslog(LOG_INFO, "Start sending messages\n");
                currentConnection->Send(&amount, sizeof(uint32_t));
                while (!m_outputMessages.empty())
                {
                    syslog(LOG_INFO, "Sended %s\n", m_outputMessages.front().m_message);
                    currentConnection->Send(&m_outputMessages.front(), sizeof(Message));
                    m_outputMessages.pop();
                }
                m_outputMessagesMutex.unlock();
                sem_post(semaphoreWrite);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(30));

            // Get all messages
            {
                {
                    timespec t;

                    clock_gettime(CLOCK_REALTIME, &t);

                    t.tv_sec += 5;

                    int s = sem_timedwait(semaphoreRead, &t);
                    if (s == -1)
                    {
                        syslog(LOG_ERR, "Read semaphore ");
                        m_isRunning = false;
                        break;
                    }
                }

                m_inputMessagesMutex.lock();
                {
                    uint32_t amount = 0;
                    currentConnection->Get(&amount, sizeof(uint32_t));
                    for (uint32_t i = 0; i < amount; i++)
                    {
                        Message msg = {0};
                        currentConnection->Get(&msg, sizeof(Message));
                        syslog(LOG_INFO, "Recieved %s\n", msg.m_message);
                        m_inputMessages.push(msg);
                    }
                }
                m_inputMessagesMutex.unlock();
            }
        }
        currentConnection->Close();
        sem_close(semaphoreWrite);
        sem_close(semaphoreRead);
        delete currentConnection;
    }
    catch (std::exception &e)
    {
        syslog(LOG_ERR, "An error %s", e.what());
    }
    catch (const char *e)
    {
        syslog(LOG_ERR, "An error %s", e);
    }
}

