#include "daemon.hpp"
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <csignal>
#include <fstream>
#include <fcntl.h>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

static void HandleSignal(int signal)
{
    switch (signal)
    {
    case SIGTERM:
        syslog(LOG_INFO, "Process terminated");
            DaemonFileRemover::GetInstance().Terminate();
        break;
    case SIGHUP:
        syslog(LOG_INFO, "Config reloading");
            DaemonFileRemover::GetInstance().ReadConfig();
        break;
    default:
        break;
    }
}

void DeleteFiles(const std::string& directory, const std::string& triggerFile)
{
    if (fs::exists(directory + '/' + triggerFile)) {
        //syslog(LOG_INFO, "Find trigger file");
        for (auto &file: fs::directory_iterator(directory)) {
            if (file.path().filename().string() != triggerFile) {
                if (!fs::remove(file)) {
                    syslog(LOG_WARNING, "Removed is failed: \"%s\"", file.path().c_str());
                }
            }
        }

    }
}

void DaemonFileRemover::KillDaemon()
{
    syslog(LOG_INFO, "Search existing daemon");
    std::ifstream pid_file(pid_path);
    if (pid_file.is_open())
    {
        int exist_daemon_pid = 0;
        if (pid_file >> exist_daemon_pid && kill(exist_daemon_pid, 0) == 0)
        {
            syslog(LOG_WARNING, "Stopping existing daemon. PID: %i", exist_daemon_pid);
            kill(exist_daemon_pid, SIGTERM);
        }
    }
}


void DaemonFileRemover::CreateDaemon()
{
    syslog(LOG_INFO, "Creating daemon start");

    pid_t pid = fork();
    if (pid < 0)
    {
        syslog(LOG_ERR, "Error: fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    umask(0);
    if (setsid() < 0)
    {
        syslog(LOG_ERR, "Error: Group setting");
        exit(EXIT_FAILURE);
    }
    if (chdir("/") < 0)
    {
        syslog(LOG_ERR, "Error: Switching to root dir");
        exit(EXIT_FAILURE);
    }
    // Close all file descriptors
    for (int tmp = sysconf(_SC_OPEN_MAX); tmp >= 0; --tmp)
    {
        close(tmp);
    }
    int devNull = open("/dev/null", O_RDWR);
    dup2(devNull, STDIN_FILENO);
    dup2(devNull, STDOUT_FILENO);
    dup2(devNull, STDERR_FILENO);
    syslog(LOG_INFO, "Creating daemon ends");
}

void DaemonFileRemover::Init(const std::string& config_local)
{
    config_abs_path = fs::absolute(config_local);

    openlog("DaemonFileRemover", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Initialization");

    KillDaemon();
    CreateDaemon();

    syslog(LOG_INFO, "Writing pid");
    std::ofstream pid_file(pid_path.c_str());
    if (!pid_file.is_open())
    {
        syslog(LOG_ERR, "Error: writing pid");
        exit(EXIT_FAILURE);
    }
    pid_file << getpid();
    syslog(LOG_INFO, "Writing pid finished");

    syslog(LOG_INFO, "Setting signal handlers");
    std::signal(SIGHUP, HandleSignal);
    std::signal(SIGTERM, HandleSignal);

    ReadConfig();
    syslog(LOG_INFO, "Initialization finished");
}

void DaemonFileRemover::Run()
{
    while (!is_Terminate)
    {
        if (!fs::is_directory(target_dir))
        {
            syslog(LOG_WARNING, "Not found target directory");
        }
        else
        {
            DeleteFiles(target_dir, trigger_File);
        }
        std::this_thread::sleep_for(sleep_time);
    }
}


void DaemonFileRemover::ReadConfig()
{
    std::ifstream config_file(config_abs_path);
    if (!config_file.is_open())
    {
        syslog(LOG_ERR, "Invalid config");
        exit(EXIT_FAILURE);
    }
    if (!std::getline(config_file, target_dir))
    {
        syslog(LOG_ERR, "Can't read directory path");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_INFO, "Load directory path: \"%s\"", target_dir.c_str());

    int sleep_seconds = -1;
    if (!(config_file >> sleep_seconds))
    {
        syslog(LOG_WARNING, "Can't read sleep time from config file");
        sleep_time = def_sleep_time;
    }
    else if (sleep_seconds <= 0)
    {
        syslog(LOG_WARNING, "Sleep time is not positive");
        sleep_time = def_sleep_time;
    }
    else
    {
        sleep_time = std::chrono::seconds(sleep_seconds);
    }
    syslog(LOG_INFO, "Sleep time set to %li seconds", sleep_time.count());
}

void DaemonFileRemover::Terminate()
{
    is_Terminate = true;
    closelog();
}