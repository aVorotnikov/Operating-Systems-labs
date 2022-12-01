#include <string>
class Daemon
{
private:
    Daemon(){};
    Daemon(const Daemon &src) = delete;
    Daemon &operator=(const Daemon &) = delete;
    void MoveFiles(void);
    void DestructOldProcess(void);
    void CreateNewProcess(void);
    void ReadConfig(void);

    const std::string PID_PATH = "/var/run/lab1.pid";
    const unsigned SLEEP_INTERVAL = 30;
    const unsigned FILE_LIFETIME_IN_MINUTES_NEEDED = 10;
    bool m_isRunning = true;
    bool m_isConfigured = false;
    std::string m_configPath;
    std::string m_dir1Path;
    std::string m_dir2Path;

public:
    void Init(const std::string &configPath);
    void Run(void);
    void Terminate(void);
    void RereadConfig(void);
    static Daemon &GetInstance(void)
    {
        static Daemon m_instance;
        return m_instance;
    }
};
