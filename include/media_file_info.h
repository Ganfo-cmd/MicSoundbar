#pragma once
#include <string>

struct MediaInfo
{
    std::string name;
    std::string path;
    double duration;
    bool available = true;
};