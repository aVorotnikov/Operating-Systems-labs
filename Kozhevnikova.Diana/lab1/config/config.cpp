#include "config.h"
#include <fstream>

bool config::read(const std::string& path, Data& data) {
    static constexpr unsigned defautDuration = 60;
    std::ifstream file(path);
    if(!file.is_open()){
        return false;
    }
    if (!std::getline(file, data.src))
        return false;

    if (!std::getline(file, data.dst))
        return false;

    if (!(file >> data.duration)) 
        data.duration = defautDuration;
    return true;
}