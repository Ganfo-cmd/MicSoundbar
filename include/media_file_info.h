#pragma once
#include <cstdint>
#include <string>

struct MediaInfo
{
    uint64_t id;
    std::string name;
    std::string path;
    double duration;
    bool available = true;
};