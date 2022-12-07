#include <string>

// Config singleton class
class Config
{
  private:
    // Path to config
    std::string configPath;
    // Path to directory to copy data from
    std::string directoryFromPath;
    // Path to directory to delete and copy data to 
    std::string directoryToPath;
    // Sleep time of the daemon by default
    const unsigned int DEFAULT_SLEEP_TIME = 30;
    // Sleep time of the daemon
    unsigned int sleepTime = DEFAULT_SLEEP_TIME;
    // Flag to know if config was parsed well
    bool isValidConfig = false;

    // Constructors and assignment operator are hidden
    Config() = default;
    Config(Config const&) = delete;
    void operator=(Config const&) = delete;
  public:
    // Function to get instance of Config class
    static Config& GetInstance() {
      static Config instance;
      return instance;
    };
    // Set config file path
    void SetConfigPath(const std::string configParse) { this->configPath = configParse; };
    // Get directroy to copy data from
    std::string GetDirectoryFromPath() { return directoryFromPath; };
    // Get directroy to delete and copy data 
    std::string GetDirectoryToPath() { return directoryToPath; };
    // Get daemon sleep time
    unsigned int GetDaemonSleepTime() { return sleepTime; };
    // Check if config is parsed well
    bool IsValid() { return isValidConfig; };
    // Function to read config file and fill Config instance data
    void ParseFile();
};