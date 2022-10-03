/// @file
/// @brief Определение класса копирователя
/// @author Воротников Андрей

#include "copier.h"

#include <fstream>
#include <sstream>

extern Copier Copier::instance;

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

Copier& Copier::GetRef()
{
    return instance;
}

bool Copier::ReadConfig(const std::string& path, std::vector<CopyInfo>& copyInfoList)
try
{
    static constexpr unsigned configLineWordsCount = 4;

    copyInfoList.clear();
    std::ifstream config(path);
    std::string line;
    while (std::getline(config, line))
    {
        if (line.empty())
            continue;
        auto splitted = SplitString(line);
        if (configLineWordsCount != splitted.size())
            return false;
        copyInfoList.push_back(CopyInfo{splitted[0], splitted[1], splitted[2], splitted[3]});
    }
    return true;
}
catch (...)
{
    return false;
}

void Copier::UpdateCopyInfo(const std::vector<CopyInfo>& copyInfoList)
{
}
