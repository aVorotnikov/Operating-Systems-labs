#include "config.h"
#include <string>
#include <fstream>

// Function to read config file and fill Config instance data
void Config::ParseFile()
{
  isValidConfig = false;
  std::ifstream file(configPath);
  // Check if file is open in stream
  if (!file.is_open()) {
    return;
  }
  // Get first folder path
  if (!std::getline(file, directoryFromPath)) {
    return;
  }
  // Get second folder path
  if (!std::getline(file, directoryToPath)) {
    return;
  }
  // Get Daemon sleep time
  if (!(file >> sleepTime)) {
    sleepTime = DEFAULT_SLEEP_TIME;
  } 
  isValidConfig = true;
}