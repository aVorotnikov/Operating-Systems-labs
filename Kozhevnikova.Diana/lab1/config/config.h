#pragma once

#include <string>

namespace config {
    struct Data {
        unsigned duration;
        std::string src;
        std::string dst;
    };

    bool read(const std::string& path, Data& data);

}