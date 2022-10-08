/// @file
/// @brief Работа с классом конфигурации
/// @author Воротников Андрей

#include "../daemon/daemon.h"
#include "config.h"

#include <fstream>
#include <sstream>

/// @brief Пространство имён конфигурации
namespace config
{

namespace
{

std::vector<std::string> SplitString(const std::string& str, char delim = ' ')
{
    std::stringstream stream(str);
    std::string item;
    std::vector<std::string> res;
    while (std::getline(stream, item, delim))
        res.push_back(std::move(item));
    return res;
}

} // anonymous namespace

bool Read(const std::string& path, std::vector<Copier::CopyInfo>& copyInfoList, unsigned& duration)
{
    static constexpr unsigned configLineWordsCount = 4;
    static constexpr unsigned durationLineWordsCount = 1;

    copyInfoList.clear();
    std::ifstream config(path);
    std::string line;

    duration = Daemon::defaultDurationInSeconds;
    try
    {
        while (std::getline(config, line))
        {
            if (line.empty())
                continue;
            auto splitted = SplitString(line);
            if (durationLineWordsCount == splitted.size())
            {
                duration = std::stoul(splitted[0]);
                continue;
            }
            if (configLineWordsCount != splitted.size())
                return false;
            copyInfoList.push_back(Copier::CopyInfo{splitted[0], splitted[1], splitted[2], splitted[3]});
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

} // namespace config
