/**
 * @file daemon.h
 * @author Baev Daniil (baev.daniil.2002@gmail.com)
 * @brief 
 * @version 0.2
 * @date 2022-11-17
 * 
 * @copyright Copyright (c) 2022
 */

#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>

/**
 * @brief Daemon singlton class
 */
class Daemon{
public:
    //Delete all another constructor
    Daemon(Daemon const&) = delete;
    Daemon& operator = (Daemon const&) = delete;
    Daemon (Daemon&&) = delete;
    Daemon& operator = (Daemon&&) = delete;


    /**
     * @brief Get the instance of object
     * 
     * @return Daemon& 
     */
    static Daemon& getInstance(){
        static Daemon instance;
        return instance;
    }

    /**
     * @brief Init method
     * 
     * @param configPath Path to config file
     */
    void init(std::string configPath);

    /**
     * @brief Load config file
     */
    void loadConfig(void);

    /**
     * @brief Function to start work
     */
    void run(void);

    /**
     * @brief Terminate daemon work 
     */
    void terminate(void);    
private:
    /**
     * @brief Value which used if config contains incorrect sleep time
     */
    std::chrono::seconds sleepTime = std::chrono::seconds(20);

    /**
     * @brief Path to PID file
     */
    const std::filesystem::path PID_PATH = std::filesystem::path{"/var/run/daemon_reminder.pid"};

    /**
     * @brief Path to config file 
     */
    std::filesystem::path configPath;

    /**
     * @brief Path to start directory 
     */
    std::filesystem::path directoryPath;

    /**
     * @brief Flag to check if need to terminate 
     */
    bool isTerminated = false;

    /**
     * @brief structure containing the necessary information about the event
     */
    struct Data {
        std::string directory; //> directory name
        std::string file;      //> file name
       
        Data(std::string dir, std::string fl):
            directory(dir), file(fl) {}
      
        Data(const Data&) = default;
        Data& operator=(const Data&) = default;
        Data (Data&&) = default;
        Data& operator = (Data&&) = default;
    };
    
    /**
     * @brief set of monitored events
     */
    std::vector<Data> data;

    

    /**
     * @brief Construct a new Daemon object
    */
    Daemon() = default;



    /**
     * @brief Check pid function for restart protection
     */
    void checkPid(void);

    /**
     * @brief Write pid to pidfile for restart protection
     */
    void writePid(void);

    /**
     * @brief Transform process into daemon 
     */
    void daemonize(void);
};