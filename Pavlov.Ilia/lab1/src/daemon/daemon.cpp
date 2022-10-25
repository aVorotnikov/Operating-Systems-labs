#include "daemon.h"

#include <fstream>
#include <string>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <csignal>
#include <filesystem>

Daemon Daemon::instance;
Daemon& Daemon::getInstance() {
  return instance;
}

void Daemon::ReadConfig() {
  std::ifstream config(path_to_config);
  if (!config.is_open()) {
    syslog(LOG_ERR, "Could not open config file");
    exit(EXIT_FAILURE);
  }
  
  if (!std::getline(config, dir1) || !std::getline(config, dir2)) {
    syslog(LOG_ERR, "Could not read config file");
    config.close();
    exit(EXIT_FAILURE);
  }
  dir1 = dir1.substr(0, dir1.size() - 1);
  config.close();
}

void Daemon::Fork() {
  syslog(LOG_INFO, "Start forking");
  auto stdin_copy = dup(STDIN_FILENO);
  auto stdout_copy = dup(STDOUT_FILENO);
  auto stderr_copy = dup(STDERR_FILENO);

  pid_t pid = fork();
  if (pid < 0) {
    syslog(LOG_ERR, "Fork error");
    exit(EXIT_FAILURE);
  }

  if (pid != 0) {
    syslog(LOG_INFO, "Kill parent process");
    exit(EXIT_SUCCESS);
  }
  umask(0);

  if (setsid() < 0) {
    syslog(LOG_ERR, "Fork error");
    exit(EXIT_FAILURE);
  }

  if (chdir("/") < 0) {
    syslog(LOG_ERR, "Fork error");
    exit(EXIT_FAILURE);
  }
  
  for (int x = sysconf(_SC_OPEN_MAX); x >= 0; --x) {
    close(x);
  }

  dup2(stdin_copy, STDIN_FILENO);
  dup2(stdout_copy, STDOUT_FILENO);
  dup2(stderr_copy, STDERR_FILENO);
  syslog(LOG_INFO, "Fork successed");
}

void SignalHandler(int signum) {
  switch (signum) {
  case SIGHUP:
    Daemon::getInstance().ReadConfig();
    break;
  case SIGTERM:
    Daemon::getInstance().Terminate();
    break;
  default:
    break;
  }
}

void Daemon::CheckPidFile() {
  syslog(LOG_INFO, "Checking pid file");
  std::ifstream pid_file(PATH_TO_PIDFILE);
  if (pid_file.is_open()) {
    pid_t pid = 0;
    if (pid_file >> pid && !kill(pid, 0)) {
      kill(pid, SIGTERM);
    }
    pid_file.close();
  }
}

void Daemon::WritePidFile() {
  syslog(LOG_INFO, "Writing pid file");
  std::ofstream pid_file(PATH_TO_PIDFILE);
  if (!pid_file.is_open()) {
    syslog(LOG_ERR, "Could not write pid file");
    exit(EXIT_FAILURE);
  }
  pid_file << getpid();
  pid_file.close();
}


void Daemon::Init(const std::string& path_to_config) {
  openlog("Copy Daemon", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);
  syslog(LOG_INFO, "Start daemon initialization");
  this->path_to_config = path_to_config;
  ReadConfig();
  CheckPidFile();
  Fork();
  WritePidFile();
  signal(SIGHUP, SignalHandler);
  signal(SIGTERM, SignalHandler);
  syslog(LOG_INFO, "Daemon successfully inited");
}

void Daemon::CopyDirectory() {
  if (!std::filesystem::is_directory(dir1)) {
    syslog(LOG_WARNING, "The first directory does not exist");
    return;
  }
  if (!std::filesystem::is_directory(dir2)) {
    syslog(LOG_WARNING, "The second directory does not exist");
    return;
  }
  for (const auto& entry : std::filesystem::directory_iterator(dir2))
    std::filesystem::remove_all(entry.path());
  std::filesystem::create_directory(std::filesystem::path(dir2) / "IMG");
  std::filesystem::create_directory(std::filesystem::path(dir2) / "OTHER");
  for (const auto& entry : std::filesystem::directory_iterator(dir1)) {
    if (entry.is_regular_file()) {
      if (entry.path().extension() == ".png")
        std::filesystem::copy(entry.path(), std::filesystem::path(dir2) / "IMG");
      else
        std::filesystem::copy(entry.path(), std::filesystem::path(dir2) / "OTHER");
    }
  }
}

void Daemon::Run() {
  syslog(LOG_INFO, "Run copy daemon");
  while (!term_request) {
    syslog(LOG_INFO, "Copy");
    CopyDirectory();
    sleep(INTERVAL);
  }
  syslog(LOG_INFO, "Terminate copy daemon");
  closelog();
}

void Daemon::Terminate() {
  term_request = true;
}