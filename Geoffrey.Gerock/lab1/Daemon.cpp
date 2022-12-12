#include <fstream>

#include "Daemon.h"
#include "Deleter.h"

void Daemon::init() noexcept {
    _setUpLogging();
    syslog(LOG_NOTICE, "Successful logging setup");

    _checkPid();
    _fork();
    if (setsid() < 0) {
        syslog(LOG_CRIT, "Couldn't create group");
        _terminate(EXIT_FAILURE);
    }

    signal(SIGTERM, _sigHandler);
    signal(SIGHUP, _sigHandler);

    _fork();
    _writePid();

    umask(0);
    if (chdir("/") != 0) {
        syslog(LOG_CRIT, "Couldn't change dir");
        _terminate(EXIT_FAILURE);
    }

    _fdRedirect();
}

void Daemon::_setUpLogging() {
    openlog(_daemonName.c_str(), LOG_PID | LOG_NDELAY, LOG_DAEMON);
    setlogmask(LOG_UPTO(LOG_NOTICE));
}

void Daemon::_checkPid() {
    std::ifstream pidFile(_pidPath);
    int pid;
    if (pidFile >> pid && !kill(pid, 0)) {
        syslog(LOG_NOTICE, "Killing the previous daemon instance (pid: %d)", pid);
        kill(pid, SIGKILL);
    }
}

void Daemon::_sigHandler(int sig) {
    switch (sig) {
        case SIGHUP:
            syslog(LOG_NOTICE, "SIGHUP was passed: rereading config");
            Daemon::readConfig();
            break;
        case SIGTERM:
            syslog(LOG_NOTICE, "SIGTERM was passed: terminating");
            Daemon::_needTerminate = true;
            break;
        default:
            syslog(LOG_ERR, "Unsubscribed signal (%d) was passed", sig);
    }
}

void Daemon::_fork() {
    pid_t pid = fork();
    if (pid < 0) {
        syslog(LOG_CRIT, "Could not fork");
        _terminate(EXIT_FAILURE);
    }
    if (pid > 0) {
        syslog(LOG_NOTICE, "Successful forking, killing parent");
        _terminate(EXIT_SUCCESS);
    }
}

void Daemon::_writePid() {
    std::ofstream pidFile(_pidPath);
    if (!pidFile.is_open()) {
        syslog(LOG_CRIT, "Error during writing to pid file");
        _terminate(EXIT_FAILURE);
    }
    pid_t pid = getpid();
    pidFile << pid;
    syslog(LOG_NOTICE, "Wrote pid (%d) to pid file (%s)", pid, _pidPath.c_str());
}

void Daemon::_fdRedirect() {
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
        close(x);
    int dv = open("/dev/null", O_RDWR);
    if (dv == -1) {
        syslog(LOG_CRIT, "Could not open /dev/null");
        _terminate(EXIT_FAILURE);
    }
    for (auto fileno: {STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO})
        dup2(dv, fileno);
    syslog(LOG_NOTICE, "Redirected streams");
}

void Daemon::setConfigPath(const fs::path& configPath) {
    if (!(exists(configPath) && fs::is_regular_file(configPath) && !fs::is_empty(configPath))) {
        syslog(LOG_CRIT, "Config file (\"%s\") doesn't exist", configPath.c_str());
        _terminate(EXIT_FAILURE);
    }
    const_cast<fs::path&>(_configPath) = fs::canonical(configPath);
    _config.setPath(configPath);
    syslog(LOG_NOTICE, "Set both the config path and the configReader up");
}

void Daemon::readConfig() noexcept {
    if (!Daemon::getInstance()._config.isInited() || Daemon::getInstance()._config.readConfig() == EXIT_FAILURE) {
        syslog(LOG_CRIT, "Error during reading the config file");
        _terminate(EXIT_FAILURE);
    }
    syslog(LOG_NOTICE, "Success on reading the config file");
}

void Daemon::run() noexcept {
    while (!_needTerminate) {
        syslog(LOG_NOTICE, "Doing job");
        _doJob();
        syslog(LOG_NOTICE, "Job done, sleeping");
        sleep(_sleepTime);
    }
    _tearDownLogging();
}

void Daemon::_doJob() {
    Deleter::deleteDirs(_config.getItems());
}

void Daemon::_tearDownLogging() {
    syslog(LOG_NOTICE, "Terminating and tearing down logging");
    closelog();
}

void Daemon::_terminate(const int status) {
    _tearDownLogging();
    exit(status);
}
