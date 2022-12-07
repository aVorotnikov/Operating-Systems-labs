#include "daemon.h"
#include "config.h"

#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <dirent.h>
#include <fstream>
#include <filesystem>
#include <csignal>
#include <unistd.h>
#include <cstring>

// Utility function to delete all files and directories in given folder
void DeleteFilesInDirectory(std::string path)
{
  for (auto& dir : std::filesystem::directory_iterator(path)) {
    std::filesystem::remove_all(dir);
  }
}

// Utility function to copy all files with specific extension from one directory to another
void CopyFilesWithExtension(std::string directoryFrom, std::string directoryTo, std::string extension)
{
  for (auto& file : std::filesystem::directory_iterator(directoryFrom)) {
    if (file.path().extension() == extension) {
      std::filesystem::copy_file(file, directoryTo / file.path().filename());
    }
  }
}

// Utility function to handle signals
static void SignalHandler(int signal)
{
  switch(signal) {
    case SIGHUP:
      Config::GetInstance().ParseFile();
      break;
    case SIGTERM:
      Daemon::GetInstance().Terminate();
      break;
    default:
      break;
  }
}

// Function to check pid file to see if daemon is already working
void Daemon::CheckPid()
{
  syslog(LOG_INFO, "Checking pid file");
  std::ifstream pidFile(PID_PATH);
  if (pidFile.is_open()) {
    int pid = 0;
    if (pidFile >> pid && kill(pid, 0) == 0) {
      syslog(LOG_INFO, "Killing another daemon instance");
      kill(pid, SIGTERM);
    }
    pidFile.close();
  }
}

// Function for deamonization of proccess
void Daemon::Fork()
{
  syslog(LOG_INFO, "Forking process");
  
  int stdinCopy = dup(STDIN_FILENO);
  int stdoutCopy = dup(STDOUT_FILENO);
  int stderrCopy = dup(STDERR_FILENO);

  pid_t pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    exit(EXIT_SUCCESS);
  }
  umask(0);
  if (setsid() < 0) {
    exit(EXIT_FAILURE);
  }
  if (chdir("/") < 0) {
    exit(EXIT_FAILURE);
  }
  for (long x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
    close(x);
  }
  
  dup2(stdinCopy, STDERR_FILENO);
  dup2(stdoutCopy, STDOUT_FILENO);
  dup2(stderrCopy, STDERR_FILENO);
}

// Function to write our process's pid to pid file
void Daemon::WriteToPID()
{
  syslog(LOG_INFO, "Writing to PID file");
  std::ofstream pidFile(PID_PATH);
  if (!pidFile.is_open()) {
    syslog(LOG_ERR, "Error while opening PID file");
    exit(EXIT_FAILURE);
  }
  pidFile << getpid();
  pidFile.close();
}

// Function to set signals
void Daemon::SetSignals()
{
  std::signal(SIGHUP, SignalHandler);
  std::signal(SIGTERM, SignalHandler);
}

// Function of main daemon logic with folders exchange
void Daemon::CopyAndDelete()
{
  if (!Config::GetInstance().IsValid()) {
    syslog(LOG_ERR, "Invalid config format");
    return;
  }

  std::string directoryFrom = Config::GetInstance().GetDirectoryFromPath();
  if (!std::filesystem::is_directory(directoryFrom)) {
    syslog(LOG_ERR, "Directory %s doesn't exist", directoryFrom.c_str());
    return;
  }
  std::string directoryTo = Config::GetInstance().GetDirectoryToPath();
  if (!std::filesystem::is_directory(directoryFrom)) {
    syslog(LOG_ERR, "Directory %s doesn't exist", directoryTo.c_str());
    return;
  }

  try {
    // Remove recursively all from second folder
    syslog(LOG_INFO, "Deleting all in %s", directoryTo.c_str());
    DeleteFilesInDirectory(directoryTo);
    // Copy all .bk files from first folder to second folder
    syslog(LOG_INFO, "Copying .bk files from %s", directoryFrom.c_str());
    CopyFilesWithExtension(directoryFrom, directoryTo, FILE_EXTENSION);
  }
  catch(std::filesystem::filesystem_error& e) {
    syslog(LOG_ERR, "%s", e.what());
  }
}

// Function to initialize Daemon instance with config file
void Daemon::Initialize(const std::string configPath)
{
  openlog("bk_copier_daemon", LOG_NDELAY | LOG_PID, LOG_USER);
  syslog(LOG_INFO, "Initializing daemon");

  Config::GetInstance().SetConfigPath(std::filesystem::absolute(configPath));
  CheckPid();
  Fork();
  WriteToPID();
  SetSignals();
  Config::GetInstance().ParseFile();
  IsWorking = true;
  syslog(LOG_INFO, "Daemon initialized");
}

// Function to stop daemon work cycle
void Daemon::Terminate()
{
  IsWorking = false;
  syslog(LOG_INFO, "Daemon terminated");
  closelog();
}

//Function to start daemon work cycle
void Daemon::Run()
{
  while (IsWorking) {
    CopyAndDelete();
    sleep(Config::GetInstance().GetDaemonSleepTime());
  }
}