#pragma once
#include <cstdint>
#include <string>

struct MediaInfo
{
    uint64_t id;
    std::string name;
    std::string path;
    uint32_t duration;
    std::string hotkey;
    bool available = true;
};

enum class SortField
{
    Name,
    Duration
};

enum class SortOrder
{
    Ascending,
    Descending
};