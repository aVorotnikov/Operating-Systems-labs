#include <sys/syslog.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include <csignal>
#include <chrono>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

#include "host.h"
#include "../connections/connection.h"

Host Host::m_hostInstance;

int main(int argc, char *argv[])
{
    openlog("lab2_host", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);

    try
    {
        Host::GetInstance().Run();
    }
    catch (std::exception &err)
    {
        syslog(LOG_ERR, "An err %s", err.what());
    }

    closelog();
    return 0;
}

void Host::SignalHandler(int signum, siginfo_t *info, void *ptr)
{
    switch (signum)
    {
        case SIGUSR1:
        {
            syslog(LOG_INFO, "Client %d request connection to host", info->si_pid);
            if (Host::GetInstance().m_clientPid == -1)
                Host::GetInstance().m_clientPid = info->si_pid;
            else

                syslog(LOG_INFO, "But host has already client %d", Host::GetInstance().GetClientPid());
            break;
        }
        case SIGTERM:
        {
            Host::GetInstance().Stop();
            break;
        }
    }
}

Host::Host(void)
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

void Host::Run(void)
{
    syslog(LOG_INFO, "Host started");
    m_isRunning = true;

    // run thread with connections
    std::thread t(&Host::ConnectionWork, this);

    while (m_isRunning.load())
    {
        if (!m_inputMessages.empty())
        {
            m_inputMessagesMutex.lock();
            printf("%s\n", m_inputMessages.front().m_message);
            m_inputMessages.pop();
            m_inputMessagesMutex.unlock();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    t.join();
}

void Host::Stop()
{
    m_isRunning = false;
    syslog(LOG_INFO, "Start terminating host");
}

Host::~Host(void)
{
}

void Host::ConnectionWork()
{
    try
    {
        printf("host pid = %i\n", getpid());
        auto lastTimeWeHadClient = std::chrono::high_resolution_clock::now();
        while (m_isRunning.load())
        {
            // if this happening for 5 minutes -> exit
            if (m_clientPid.load() == -1)
            {
                auto minutesPassed = std::chrono::duration_cast<std::chrono::minutes>(
                    std::chrono::high_resolution_clock::now() - lastTimeWeHadClient).count();

                if (minutesPassed >= 60)
                {
                    // we should stop our work
                    Stop();
                }
                continue;
            }
            lastTimeWeHadClient = std::chrono::high_resolution_clock::now();

            syslog(LOG_INFO, "Start creating connection for client %d", GetClientPid());
            Connection *currentConnection = Connection::CreateConnection(m_clientPid, true);
            if (!currentConnection)
            {
                syslog(LOG_ERR, "Connection creation error");
                m_clientPid = -1;
                continue;
            }

            // open semaphores
            sem_t *semaphoreRead, *semaphoreWrite;
            std::string semNameRead = "/host_" + std::to_string(m_clientPid);
            std::string semNameWrite = "/client_" + std::to_string(m_clientPid);
            semaphoreRead = sem_open(semNameRead.c_str(), O_CREAT | O_EXCL, 0777, 0);
            if (semaphoreRead == SEM_FAILED)
            {
                syslog(LOG_ERR, "Semaphore creation error");
                delete currentConnection;
                m_clientPid = -1;
                continue;
            }
            syslog(LOG_INFO, "created semaphore %s", semNameRead.c_str());
            semaphoreWrite = sem_open(semNameWrite.c_str(), O_CREAT | O_EXCL, 0777, 0);
            if (semaphoreWrite == SEM_FAILED)
            {
                syslog(LOG_ERR, "Semaphore creation error");
                sem_close(semaphoreRead);
                delete currentConnection;
                m_clientPid = -1;
                continue;
            }
            syslog(LOG_INFO, "created semaphore %s", semNameWrite.c_str());


            // send signal that we are ready
            if (kill(m_clientPid, SIGUSR1) != 0)
            {
                syslog(LOG_ERR, "Cannot send signal to %d", GetClientPid());
            }
            syslog(LOG_INFO, "Signal sent");

            currentConnection->Open(0, true);
            syslog(LOG_INFO, "Opened connection");

            auto clock = std::chrono::high_resolution_clock::now();

            // while m_isRunning and time between messages are
            while (m_isRunning.load())
            {
                //printf("%ld %ld\n", m_inputMessages.size(), m_outputMessages.size());
                double minutes_passed =
                    std::chrono::duration_cast<std::chrono::minutes>(
                        std::chrono::high_resolution_clock::now() - clock).count();

                if (minutes_passed >= 1)
                {
                    syslog(LOG_INFO, "Killing client for 1 minute silence");
                    kill(m_clientPid, SIGTERM);
                    m_clientPid = -1;
                    break;
                }

                // Get all messages
                {
                    {
                        timespec t;

                        clock_gettime(CLOCK_REALTIME, &t);

                        t.tv_sec += 5;

                        int s = sem_timedwait(semaphoreRead, &t);
                        if (s == -1)
                        {
                            kill(m_clientPid, SIGKILL);
                            m_clientPid = -1;
                            syslog(LOG_ERR, "Read semaphore timeout");
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
                            printf("Recieved %s\n", msg.m_message);
                            m_inputMessages.push(msg);
                            // we got a message
                            clock = std::chrono::high_resolution_clock::now();
                        }
                    }
                    m_inputMessagesMutex.unlock();
                }
            }

            // Send all messages
            {
                m_outputMessagesMutex.lock();
                uint32_t amount = m_outputMessages.size();
                currentConnection->Send(&amount, sizeof(uint32_t));
                while (!m_outputMessages.empty())
                {
                    syslog(LOG_INFO, "Sended %s", m_outputMessages.front().m_message);
                    currentConnection->Send(&m_outputMessages.front(), sizeof(Message));
                    m_outputMessages.pop();
                }
                m_outputMessagesMutex.unlock();
                sem_post(semaphoreWrite);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(30)); // for client get semaphore

            currentConnection->Close();
            sem_close(semaphoreWrite);
            sem_close(semaphoreRead);
            delete currentConnection;
        }
        if (m_clientPid != -1)
        {
            kill(m_clientPid, SIGTERM);
        }
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