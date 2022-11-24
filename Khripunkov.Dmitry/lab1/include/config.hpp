#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

class Config {
  public:
    Config() = default;
    Config(const std::string &filepath) { read(filepath); }

    void read(const std::string &filepath) {
        entries.clear();

        std::ifstream f(filepath);
        std::string dir1, dir2, time;

        while (f >> dir1 >> dir2 >> time) {
            entries.push_back(std::make_tuple(std::filesystem::path(dir1), std::filesystem::path(dir2),
                                              std::time_t(std::stoi(time))));
        }
    }

    inline const std::vector<std::tuple<std::filesystem::path, std::filesystem::path, std::time_t>> &
    get_entries() const {
        return entries;
    }

  private:
    std::vector<std::tuple<std::filesystem::path, std::filesystem::path, std::time_t>> entries;
};
