/// @file
/// @brief Определение класса копирователя
/// @author Воротников Андрей

#include "copier.h"

#include <list>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

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

bool Copier::UpdateCopyInfo(const std::vector<CopyInfo>& copyInfoList)
{
    std::list<std::string> sources;
    std::map<std::string, DstDirectory> dstDirectories;
    for (const auto& copyInfo : copyInfoList)
    {
        auto dstDir = dstDirectories.find(copyInfo.dst);
        if (dstDirectories.end() == dstDir)
        {
            dstDirectories[copyInfo.dst] = DstDirectory();
            dstDir = dstDirectories.find(copyInfo.dst);
        }
        auto& dstDirContent = dstDir->second;
        auto srcDir = dstDirContent.find(copyInfo.src);
        if (dstDirContent.end() == srcDir)
        {
            dstDirContent[copyInfo.src] = ExtSubfolders();
            srcDir = dstDirContent.find(copyInfo.src);
        }
        auto& extSubfolders = srcDir->second;
        std::string dottedExtension = "." + copyInfo.extension;
        auto extension = extSubfolders.find(dottedExtension);
        if (extSubfolders.end() == extension)
            extSubfolders[dottedExtension] = {copyInfo.subfolder};
        else
            extension->second.insert(copyInfo.subfolder);
    }
    for (const auto& dst : dstDirectories)
        if (sources.cend() != std::find(sources.cbegin(), sources.cend(), dst.first))
            return false;
    dstDirectories_ = dstDirectories;
    return true;
}

bool Copier::Copy()
{
    const std::set<std::string> othersSubdirs = {othersSubdirectory};

    for (const auto& [dstDirName, dstDirContent] : dstDirectories_)
    {
        auto dstDirPath = std::filesystem::path(dstDirName);
        std::filesystem::remove_all(dstDirPath);
        std::filesystem::create_directory(dstDirPath);
        for (const auto& [srcDirName, srcDirContent] : dstDirContent)
            for (const auto& entry : std::filesystem::directory_iterator(srcDirName))
            {
                if (!entry.is_regular_file())
                    continue;
                auto path = entry.path();
                auto subdirs = &othersSubdirs;
                std::string a = path.extension();
                auto extensionIt = srcDirContent.find(path.extension());
                if (srcDirContent.end() != extensionIt)
                    subdirs = &extensionIt->second;
                for (const auto& subdir : *subdirs)
                {
                    auto subdirPath = dstDirPath / subdir;
                    std::filesystem::create_directory(subdirPath);
                    std::filesystem::copy_file(path, subdirPath / path.filename(), std::filesystem::copy_options::skip_existing);
                }
            }
    }
    return true;
}
