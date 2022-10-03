#include "daemon.h"
#include <unistd.h>
#include <csignal>
#include <syslog.h>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <fstream>

// initialization of singleton
Daemon Daemon::m_daemonInstance;

/// Utility functions
// check if directory exist
bool DirExists(const char *path)
{
    struct stat info;
    return stat( path, &info ) == 0 &&
        (info.st_mode & S_IFDIR);
}

// Callback for signal handle
static void SignalHandler(int sig)
{
    switch(sig)
    {
        case SIGHUP:
            Daemon::GetInstance().ReloadConfig();
            break;
        case SIGTERM:
            Daemon::GetInstance().Terminate();
            break;
        default:
            break;
    }
}

/// Daemon private methods
void Daemon::StartDaemon(void)
{
    pid_t pid;

    // store duplicates of std descriptors
    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);
    int stderr_copy = dup(STDERR_FILENO);

    // 1) Process creation
    /* Fork off the parent process */
    pid = fork();

    // 2) Parent closing
    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    // 3) Set new process group
    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    // 4.1) change working dir
    if (chdir("/") < 0)
        exit(EXIT_FAILURE);

    // 4.2) Close all opened file descriptors
    for (int x = sysconf(_SC_OPEN_MAX); x>=0; x--)
        close (x);

    // 5) Open file descriptors for stdin/stdout/stderr
    dup2(stdin_copy, STDIN_FILENO);
    dup2(stdout_copy, STDOUT_FILENO);
    dup2(stderr_copy, STDERR_FILENO);
}

void Daemon::ParseConfig(void)
{
    syslog(LOG_INFO, "Parse config");
    m_sleepTime = DEFAULT_SLEEP_TIME;

    std::ifstream configFile(m_configPath);
    // open file
    if (!configFile.is_open())
    {
        syslog(LOG_INFO, "Invalid config. Not doing anything");
        return;
    }

    // Get target directory path
    if (!std::getline(configFile, m_cleanPath))
    {
        syslog(LOG_INFO, "Cannot read file path. Not doing anything");
        return;
    }

    // Get sleep in seconds
    if (!(configFile >> m_sleepTime))
    {
        syslog(LOG_INFO, "Cannot read sleep in seconds. Not doing anything");
        return;
    }

    syslog(LOG_INFO, "Config loaded successfully");
    m_isValidConfig = true;
}

/// Daemon public methods
void Daemon::Init(std::string configFilePath)
{
    char buf[PATH_MAX];
    getcwd(buf, sizeof(buf));
    m_configPath = buf;
    m_configPath += "/" + configFilePath;

    // init logger
    openlog("FileDeleterDaemon", LOG_NDELAY | LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Daemon initialization");

    // check if daemon already running
    {
        std::ifstream pidFile(PID_FILE_PATH);
        syslog(LOG_INFO, "Check PidFile");
        if (pidFile.is_open())
        {
            int otherDaemonPid = 0;
            if (pidFile >> otherDaemonPid)
            {
                // check if process exist
                if (kill(otherDaemonPid, 0) == 0)
                {
                    syslog(LOG_INFO, "Another instance in running, terminating it...");
                    // kill it
                    kill(otherDaemonPid, SIGTERM);
                }
            }
        }
    }

    syslog(LOG_INFO, "Daemon daemonification");
    StartDaemon();

    // write to pidFile our Pid
    {
        std::ofstream pidFile(PID_FILE_PATH.c_str());
        if (!pidFile.is_open())
        {
            syslog(LOG_INFO, "Cannot write to Pid file, exiting");
            exit(EXIT_FAILURE);
        }

        pidFile << getpid();
    }

    // set signal handler
    syslog(LOG_INFO, "Set signal handlers");
    signal(SIGHUP, SignalHandler);
    signal(SIGTERM, SignalHandler);

    // Parse config
    ParseConfig();
}

void Daemon::ReloadConfig(void)
{
    syslog(LOG_INFO, "Config reloading");
    m_isValidConfig = false;
    ParseConfig();
}

void Daemon::Terminate(void)
{
    syslog(LOG_INFO, "Process terminated");
    m_isRunning = false;
    // Close opened logger
    closelog();
}

void Daemon::Run(void)
{
    while (m_isRunning)
    {
        DoWork();
        sleep(m_sleepTime);
    }
}

void Daemon::DoWork(void)
{
    // if config not valid - do not do anything
    if (!m_isValidConfig)
        return;

    // check if directory exist
    if (!DirExists(m_cleanPath.c_str()))
    {
        syslog(LOG_INFO, "Directory %s not exist", m_cleanPath.c_str());
        return;
    }

    std::vector<std::string> subfiles;

    // collect all subfiles
    syslog(LOG_INFO, "Collect all subfiles in directory");
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;
    dp = opendir(m_cleanPath.c_str());
    if (dp != nullptr)
    {
        while ((entry = readdir(dp)))
        {
            subfiles.push_back(entry->d_name);
        }
    }
    closedir(dp);

    // iterate over all subfiles
    for (auto &name : subfiles)
    {
        std::string fullname = m_cleanPath + "/" + name;
        syslog(LOG_INFO, "Remove %s", fullname.c_str());

        // if this is a directory
        if (name != "." && name != ".." && DirExists(fullname.c_str()))
            system(("rm -rf \"" + fullname + "\"").c_str());
    }
}
