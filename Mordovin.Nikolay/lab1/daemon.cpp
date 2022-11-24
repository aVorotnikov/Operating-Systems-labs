#include "daemon.h"
#include <limits.h>
#include <unistd.h>
#include <syslog.h>
#include <csignal>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <chrono>

static void SignalHandler(int signal)
{
    switch (signal)
    {
    case SIGTERM:
        syslog(LOG_INFO, "Terminating process");
        Daemon::GetInstance().Terminate();
        break;
    case SIGHUP:
        syslog(LOG_INFO, "Reading config");
        Daemon::GetInstance().RereadConfig();
        break;

    default:
        break;
    }
}

void Daemon::DestructOldProcess(void)
{
    syslog(LOG_INFO, "Checking old pid");
    std::ifstream pidFile(PID_PATH);

    if (pidFile.is_open())
    {
        int oldPid = 0;
        if (pidFile >> oldPid)
        {
            if (kill(oldPid, 0) == 0)
            {
                syslog(LOG_INFO, "Old process is running. Killing it");
                kill(oldPid, SIGTERM);
            }
        }
    }
    else
    {
        syslog(LOG_INFO, "Unable to open pid file");
    }
}

void Daemon::CreateNewProcess(void)
{
    syslog(LOG_INFO, "Creating new process");

    pid_t pid = fork();

    if (pid < 0)
    {
        syslog(LOG_ERR, "Fork error!");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    if (setsid() < 0)
    {
        syslog(LOG_ERR, "Process group setting error!");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") < 0)
    {
        syslog(LOG_ERR, "Failed to change to root directory");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Writing pid in file");
    std::ofstream pidFile(PID_PATH);

    if (!pidFile.is_open())
    {
        syslog(LOG_ERR, "Unable to open pid file");
        exit(EXIT_FAILURE);
    }

    pidFile << getpid();

    signal(SIGHUP, SignalHandler);
    signal(SIGTERM, SignalHandler);

    dup2(open("/dev/null", O_RDONLY), STDIN_FILENO);
    dup2(open("/dev/null", O_WRONLY), STDOUT_FILENO);
    dup2(open("/dev/null", O_WRONLY), STDERR_FILENO);
}

void Daemon::ReadConfig(void)
{
    syslog(LOG_INFO, "Reading config file");

    std::ifstream configFile(m_configPath);

    if (!configFile.is_open())
    {
        syslog(LOG_WARNING, "Unable to open config file");
        return;
    }

    // read paths from config file
    std::string path1, path2;

    if (!std::getline(configFile, path1) || !std::getline(configFile, path2))
    {
        syslog(LOG_WARNING, "Wrong config file");
        return;
    }

    syslog(LOG_INFO, "Checking if these directories exist");

    if (std::filesystem::is_directory(path1) && std::filesystem::is_directory(path2))
    {
        syslog(LOG_INFO, "Directories found");
        m_dir1Path = path1.c_str();
        m_dir2Path = path2.c_str();
    }
    else
    {
        syslog(LOG_WARNING, "Directories not found");
        return;
    }

    m_isConfigured = true;

    syslog(LOG_INFO, "Config file readed successfully");
}

void Daemon::RereadConfig(void)
{
    m_isConfigured = false;
    ReadConfig();
}

void Daemon::Init(const std::string &configPath)
{
    char buf[PATH_MAX];
    getcwd(buf, sizeof(buf));
    m_configPath = buf + std::string("/") + configPath;

    openlog("MordovinNikDaemon", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "initializating daemon");

    DestructOldProcess();
    CreateNewProcess();

    ReadConfig();
}

void Daemon::Run(void)
{
    syslog(LOG_INFO, "Started moving files");

    while (m_isRunning)
    {
        MoveFiles();
        sleep(SLEEP_INTERVAL);
    }
}

void Daemon::Terminate(void)
{
    m_isRunning = false;
    closelog();
}

void Daemon::MoveFiles(void)
{
    if (!m_isConfigured)
    {
        syslog(LOG_WARNING, "No correct configuration provided");
        return;
    }

    auto time_now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    struct stat buff;
    std::vector<std::filesystem::path> filesToMove1, filesToMove2;

    for (const auto &entry : std::filesystem::directory_iterator(m_dir1Path))
    {
        if (!entry.is_directory())
        {
            stat(entry.path().c_str(), &buff);
            if ((time_now - timelocal(localtime(&buff.st_mtime))) / 60 > FILE_LIFETIME_IN_MINUTES_NEEDED)
                filesToMove1.push_back(entry.path());
        }
    }

    for (const auto &entry : std::filesystem::directory_iterator(m_dir2Path))
    {
        if (!entry.is_directory())
        {
            stat(entry.path().c_str(), &buff);
            if ((time_now - timelocal(localtime(&buff.st_mtime))) / 60 < FILE_LIFETIME_IN_MINUTES_NEEDED)
                filesToMove2.push_back(entry.path());
        }
    }

    for (auto &filePath : filesToMove1)
        std::filesystem::rename(filePath, m_dir2Path / filePath.filename());

    for (auto &filePath : filesToMove2)
        std::filesystem::rename(filePath, m_dir1Path / filePath.filename());

    syslog(LOG_INFO, "Files moved successfully");
}
