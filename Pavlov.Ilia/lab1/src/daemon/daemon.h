#pragma once

#include <string>
#include <filesystem>

class Daemon {
private:
  const std::string PATH_TO_PIDFILE = "var/run/lab1.pid";
  const std::string DIRECTORY_FOR_IMAGES = "IMG";
  const std::string DIRECTORY_FOR_OTHER_FILES = "OTHER";
  const unsigned int INTERVAL = 60;
  static Daemon instance;
  std::filesystem::path path_to_config;
  std::string dir1;
  std::string dir2;
  bool term_request = false;
  Daemon() = default;
  void ReadConfig();
  void Fork();
  void CheckPidFile();
  void WritePidFile();
  void Terminate();
  void CopyDirectory();
  friend void SignalHandler(int signum);
public:
  Daemon(const Daemon&) = delete;
  Daemon& operator=(const Daemon&) = delete;
  static Daemon& getInstance();
  void Init(const std::string& path_to_config);
  void Run();
};