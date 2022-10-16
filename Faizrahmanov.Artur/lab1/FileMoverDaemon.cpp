#include "FileMoverDaemon.h"
#include "Config.h"

#include <csignal>
#include <fstream>
#include <filesystem>

FileMoverDaemon *FileMoverDaemon::instance = nullptr;

FileMoverDaemon::FileMoverDaemon()
{
}

FileMoverDaemon::~FileMoverDaemon()
{
    delete instance;
}

FileMoverDaemon *FileMoverDaemon::getInstance()
{
    if (instance == nullptr)
    {
        instance = new FileMoverDaemon();
    }

    return instance;
}

void signalHandler(int signal)
{
    switch (signal)
    {
    case SIGTERM:
        syslog(LOG_INFO, "Process terminated");
        FileMoverDaemon::getInstance()->stop();
        closelog();
        break;
    case SIGHUP:
        syslog(LOG_INFO, "Read config");
        if (!Config::getInstance()->readConfig())
            syslog(LOG_INFO, "Config is not in the correct format");
        break;
    default:
        syslog(LOG_INFO, "Unknown signal found");
        break;
    }
}

void FileMoverDaemon::initialize(const std::string &configPath)
{
    openlog("FileMoverDaemon", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Read config");
    Config::getInstance()->setConfigPath(configPath);
    if (!Config::getInstance()->readConfig())
        syslog(LOG_INFO, "Config is not in the correct format");

    destructOldPid();
    createPid();

    isRunning = true;
}

void FileMoverDaemon::createPid()
{
    syslog(LOG_INFO, "Forking");

    pid_t pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    if ((chdir("/")) < 0)
    {
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Write new pid");
    std::ofstream file(PID_PATH);
    if (!file.is_open())
    {
        syslog(LOG_ERR, "Can't open pid file");
        exit(EXIT_FAILURE);
    }
    file << getpid();
    file.close();

    signal(SIGHUP, signalHandler);
    signal(SIGTERM, signalHandler);
}

void FileMoverDaemon::destructOldPid()
{
    syslog(LOG_INFO, "Destruct old pid if needed");
    std::ifstream file;

    file.open(PID_PATH);

    if (!file)
    {
        syslog(LOG_INFO, "Can't check that pid already exists");
        return;
    }

    int pid;
    if (file >> pid)
    {
        if (kill(pid, 0) == 0)
            kill(pid, SIGTERM);
    }

    file.close();
}

void FileMoverDaemon::run()
{
    while (isRunning)
    {
        moveFiles();
        sleep(Config::getInstance()->getSleepDuration());
    }
}

void FileMoverDaemon::moveFiles()
{
    syslog(LOG_INFO, "Start move files");

    if (Config::getInstance()->isConfigReaded())
    {
        do
        {
            std::string fromPath = Config::getInstance()->getFromPath();
            std::string toPath = Config::getInstance()->getToPath();
            std::string ext = Config::getInstance()->getFileExt();

            if (pathExist(fromPath) && pathExist(toPath))
            {
                syslog(LOG_INFO, "Move files with %s extension from %s to %s", ext.c_str(), fromPath.c_str(), toPath.c_str());
                std::string command = "mv " + std::filesystem::absolute(fromPath).string() 
                            + "/*." + ext + " " + std::filesystem::absolute(toPath).string();
                system(command.c_str());
            }

        } while (Config::getInstance()->next());
    }
}

bool FileMoverDaemon::pathExist(const std::string &path) const
{
    return std::filesystem::exists(path);
}

void FileMoverDaemon::stop()
{
    isRunning = false;
}