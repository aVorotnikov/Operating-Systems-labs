#include <string>
// Daemon singleton class
class Daemon
{
  private:
    // Path to pid file
    static constexpr const char* PID_PATH = "/var/run/bk_copier_daemon.pid";
    // Extension of files to copy
    static constexpr const char* FILE_EXTENSION = ".bk";
    // Flag to check daemon condition
    bool IsWorking = false;
    // Function to check pid file to see if daemon is already working
    void CheckPid();
    // Function for deamonization of proccess
    void Fork();
    // Function to write our process's pid to pid file
    void WriteToPID();
    // Function to set signals
    void SetSignals();
    // Function of main daemon logic with folders exchange
    void CopyAndDelete();

    // Function to check
    // Constructors and assignment operator are hidden
    Daemon() = default;
    Daemon(Daemon const&) = delete;
    void operator=(Daemon const&) = delete;
  public:
    // Function to get instance of Daemon class
    static Daemon& GetInstance() {
        static Daemon instance;
        return instance;
    };
    // Function to initialize Daemon instance with config file
    void Initialize(const std::string configPath);
    // Function to stop daemon work cycle
    void Terminate();
    // Function to start daemon work cycle
    void Run();
};