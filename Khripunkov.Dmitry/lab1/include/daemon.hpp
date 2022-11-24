#pragma once
#include "config.hpp"

class Daemon {
  public:
    static Daemon &create(const std::string &path_to_config);

    void start();

    Daemon() = delete;
    Daemon(const Daemon &) = delete;
    Daemon(Daemon &&) = delete;

  protected:
    static void signal_handler(int sig);

    void close_running();
    void daemonize();

  private:
    Daemon(const std::string &path_to_config);

    static std::string config_path;
    static Config config;
    std::string pid_file_path;
    const std::string proc_dir = "/proc";
    const std::string syslog_proc_name = "mydaemon";
};
