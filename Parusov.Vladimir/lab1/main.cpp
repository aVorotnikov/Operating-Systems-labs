#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>

const char *const PID_FILE = "/var/run/lab1.pid";

void SignalHandler(int sig);

// singleton Daemon representation class
class Daemon
{
  private:
    // Function to transform our process into Deamon
    void StartDaemon(void)
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

    // path to PID file
    std::string pidFilePath = "/var/run/lab1.pid";
    // path to config file
    std::string configPath;
    bool isRunning = true;

    bool isValidConfig = false;
    unsigned int sleepTime = 10;
    std::string cleanPath;


    void ParseConfig(void)
    {
      syslog(LOG_INFO, "Parse config");
      FILE *configFile;
      // open file
      if ((configFile = fopen(configPath.c_str(), "r")) == NULL)
      {
        syslog(LOG_INFO, "Invalid config. Not doing anything");
        return;
      }

      // Get target directory path
      char cleanDirPath[300];
      if (fgets(cleanDirPath, sizeof(cleanDirPath), configFile) == NULL)
      {
        syslog(LOG_INFO, "Cannot read file path. Not doing anything");
        fclose(configFile);
        return;
      }

      cleanPath = cleanDirPath;

      cleanPath.pop_back(); // remove '\n'

      // Get sleep in seconds
      if (fscanf(configFile, "%u", &sleepTime) < 0)
      {
        syslog(LOG_INFO, "Cannot read sleep in seconds. Not doing anything");
        fclose(configFile);
        return;
      }

      syslog(LOG_INFO, "Config loaded successfully");
      isValidConfig = true;
      fclose(configFile);
    }

    // check if directory exist
    int DirExists(const char *path)
    {
      struct stat info;

      if(stat( path, &info ) != 0)
        return 0;
      else if(info.st_mode & S_IFDIR)
        return 1;
      else
        return 0;
    }
  public:
    Daemon(bool demonify, std::string configFilePath)
    {
      char buf[300];
      getcwd(buf, sizeof(buf));
      configPath = buf;
      configPath += "/" + configFilePath;

      // init logger
      openlog("FileDeleterDaemon", LOG_NDELAY | LOG_PID, LOG_USER);
      syslog(LOG_INFO, "Daemon initialization");

      // check if deamon already running
      FILE *pidFile;
      syslog(LOG_INFO, "Check PidFile");
      if ((pidFile = fopen(pidFilePath.c_str(), "r")) != NULL)
      {
        int otherDaemonPid = 0;
        if (fscanf(pidFile, "%d", &otherDaemonPid) >= 0)
        {
          // check if process exist
          if (kill(otherDaemonPid, 0) == 0)
          {
            syslog(LOG_INFO, "Another instance in running, terminating it...");
            // kill it
            kill(otherDaemonPid, SIGTERM);
          }
        }
        fclose(pidFile);
      }


      if (demonify)
      {
        syslog(LOG_INFO, "Daemon daemonification");
        StartDaemon();
      }

      // write to pidFile our Pid
      if ((pidFile = fopen(pidFilePath.c_str(), "w")) == NULL)
      {
        syslog(LOG_INFO, "Cannot write to Pid file, exiting");
        exit(EXIT_FAILURE);
      }

      fprintf(pidFile, "%d", getpid());
      fclose(pidFile);

      // set signal handler
      syslog(LOG_INFO, "Set signal handlers");
      signal(SIGHUP, SignalHandler);
      signal(SIGTERM, SignalHandler);

      // Parse config
      ParseConfig();
    }

    void ReloadConfig(void)
    {
      syslog(LOG_INFO, "Config reloading");
      isValidConfig = false;
      ParseConfig();
    }

    void Terminate(void)
    {
      syslog(LOG_INFO, "Process terminated");
      isRunning = false;
    }

    void Run(void)
    {
      while (isRunning)
      {
        DoWork();
        sleep(sleepTime);
      }
    }

    void DoWork(void)
    {
      // if config not valid - do not do anything
      if (!isValidConfig)
        return;

      // check if directory exist
      if (!DirExists(cleanPath.c_str()))
      {
        syslog(LOG_INFO, "Directory %s not exist", cleanPath.c_str());
        return;
      }

      std::vector<std::string> subfiles;

      // collect all subfiles
      syslog(LOG_INFO, "Collect all subfiles in directory");
      struct dirent *entry = nullptr;
      DIR *dp = nullptr;
      dp = opendir(cleanPath.c_str());
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
        std::string fullname =cleanPath + "/" + name;
        syslog(LOG_INFO, "Remove %s", fullname.c_str());

        // if this is a directory
        if (name != "." && name != ".." && DirExists(fullname.c_str()))
          system(("rm -rf \"" + fullname + "\"").c_str());
      }
    }


    ~Daemon()
    {
      syslog(LOG_INFO, "Daemon destructor");
      // Close opened logger
      closelog();
    }
};

Daemon *Dimooooooon;

// Callback for signal handle
void SignalHandler(int sig)
{
  switch(sig)
  {
    case SIGHUP:
      Dimooooooon->ReloadConfig();
      break;
    case SIGTERM:
      Dimooooooon->Terminate();
      break;
    default:
      break;
  }
}

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printf("Incorrect usage. Please provide only 1 argument with configuration file\n");
    return EXIT_FAILURE;
  }

  Dimooooooon = new Daemon(true, argv[1]);

  Dimooooooon->Run();

  delete Dimooooooon;

  return EXIT_SUCCESS;
}
