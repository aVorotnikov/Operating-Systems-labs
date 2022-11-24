#include "Daemon.hpp"

void Daemon::loadConfig() {
    syslog(LOG_INFO, "Parsing config file...");
    std::map<Parser::Grammar::ConfigParams, std::string> config;
    Parser& parser = Parser::getInstance();
    parser.parseConfig(_configFile);
    config = parser.getParams();

    _sleepTime = std::stoi(config[Parser::Grammar::ConfigParams::TIME_DELAY]);
    _inputDir = getAbsolutePath(config[Parser::Grammar::ConfigParams::DIRECTORY1]);
    _outputDir = getAbsolutePath(config[Parser::Grammar::ConfigParams::DIRECTORY2]);
    totalLogPath = _outputDir + "/total.log";
}

Daemon& Daemon::getInstance(){
        static Daemon _instance;
        return _instance;
}

string Daemon::getAbsolutePath(const string &path) const{
    if (path.empty() || path[0] == '/') {
        return path;
    }
    string output = _homeDir + "/" + path;
    char *result = realpath(output.c_str(), nullptr);
    if (result == nullptr) {
        throw std::runtime_error("ERROR: Couldn't find path:" + path);
    }
    string resultPath(result);
    free(result);
    return resultPath;
}

void Daemon::signalHandler(int signalNum) {
    syslog(LOG_INFO, "Handle signal %i", signalNum);
    switch (signalNum) {
        case SIGHUP:
            syslog(LOG_INFO, "Updating config file");
            getInstance().loadConfig();
            break;
        case SIGTERM:
            syslog(LOG_INFO, "Terminate daemon");
            getInstance().terminate();
            break;
        default:
            syslog(LOG_INFO, "Signal %i is not handled", signalNum);
    }
}

void Daemon::initSignals() {
    syslog(LOG_INFO, "Signals initialization");

    signal(SIGHUP, signalHandler);
    signal(SIGTERM, signalHandler);
}

void Daemon::terminate() {
    _isRunning = false;
    unlink(_pidFilePath.c_str());
    closelog();
}

void Daemon::init(const std::string &config) {
    openlog("daemonlab1", LOG_PID|LOG_NDELAY, LOG_USER);
    syslog(LOG_INFO, "Initializing daemon");

    char buf[FILENAME_MAX];
    if (!getcwd(buf, FILENAME_MAX)) {
        throw std::runtime_error("ERROR: Failed in getcwd");
    }
    _homeDir = buf;
    syslog(LOG_INFO, "Home directory - %s", buf);

    if(!initTread()){
        return;
    }
    initSignals();
    setConfig(config);
    loadConfig();

    syslog(LOG_INFO, "Daemon is successfully initialized");
    _isRunning = true;
}

void Daemon::setConfig(const std::string &configFile) {
    _configFile = getAbsolutePath(configFile);
    if (_configFile.empty()) {
        throw std::runtime_error("ERROR: Wrong config file full path " +  _configFile);
    }
    syslog(LOG_INFO, "Trying read config file - %s", _configFile.c_str());
}

bool Daemon::initTread() const{
    syslog(LOG_INFO, "Starting init thread...");
    pid_t pid_t = fork();
    if (pid_t == -1) {
        throw std::runtime_error("Fork failed");
    } 
    else if (pid_t == 0) {
        return initPid();
    }
    return false;
}

bool Daemon::initPid() const{
    if (setsid() == -1) {
        throw std::runtime_error("ERROR: Setsid return error");
    }
    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("ERROR: Fork failed");
    }
    else if(pid != 0){
        return false;
    }

    umask(0);
    if (chdir("/") == -1) {
        throw std::runtime_error("ERROR: Chdir return error: %d");
    }
    if (close(STDIN_FILENO) == -1 || close(STDOUT_FILENO) == -1 || close(STDERR_FILENO) == -1) {
        throw std::runtime_error("Close return error: %d");
    }

    checkPid();
    return true;
}

void Daemon::checkPid() const{
    syslog(LOG_INFO, "Handle PID file...");
    std::ifstream pidFile(_pidFilePath);
    if (!pidFile.is_open()) {
        throw std::runtime_error("ERROR: Can't open pid file");
    }

    pid_t newPidFile;
    pidFile >> newPidFile;
    pidFile.close();

    std::string path = "/proc/" + std::to_string(newPidFile);
    if (std::filesystem::exists(path)) {
        kill(newPidFile, SIGTERM);
    }

    savePid();
}

void Daemon::savePid() const{
    syslog(LOG_INFO, "Saving pid file...");
    std::ofstream out(_pidFilePath);
    if (!out.is_open()) {
        throw std::runtime_error("ERROR: Can't open pid file");
    }
    out << getpid();
    out.close();
    syslog(LOG_NOTICE, "Thread init complete");
}

void Daemon::run(){
    while (_isRunning) {
        walkThroughFile(_inputDir);
        sleep(_sleepTime);
    }
}

void Daemon::walkThroughFile(const string& path){
    for (auto&& file : std::filesystem::directory_iterator(path)) {
        string filePath = file.path().string();

        if (std::filesystem::is_directory(file)) {
            walkThroughFile(filePath);
        }
        else if(isLogFile(filePath)){
            try{
                copyContent(filePath);
            }
            catch(std::exception &ex){
                std::cerr << ex.what();
            }
        }
    }
}

void Daemon::copyContent(const string& filePath) const{
        if(!std::filesystem::exists(filePath)) {
            throw std::runtime_error("ERROR: Undefined file path");
        }
        std::ofstream fout(totalLogPath, std::ios_base::out | std::ios_base::app);
        std::ifstream fin(filePath);
        fout << "\n\n" << filePath + "\n" << std::endl;

        string line;
        while (std::getline(fin, line)) {
            fout << line + "\n";
        }
        fin.close();

        if(!std::filesystem::remove(filePath)){
            throw std::runtime_error("ERROR: deleting a file");
        }
}

bool Daemon::isLogFile(const string& file) const{
    std::regex reg("." + targetFileFormat + "$");
    return regex_search(file, reg);
}