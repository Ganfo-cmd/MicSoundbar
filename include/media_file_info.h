#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct Hotkey
{
    uint32_t scan_code = 0;
    uint32_t modifiers = 0;
    std::string display;

    bool operator==(const Hotkey &other) const
    {
        return scan_code == other.scan_code && modifiers == other.modifiers;
    }

    bool IsEmpty() const
    {
        return scan_code == 0 && modifiers == 0;
    }
};

struct MediaInfo
{
    uint64_t id;
    std::string name;
    std::string path;
    uint32_t duration;
    Hotkey hotkey;
    bool available = true;
};

struct MediaList
{
    uint64_t id;
    std::string name;
    std::vector<MediaInfo> media;
};

enum class SortField
{
    Name,
    Duration,
    Hotkey
};

enum class SortOrder
{
    Ascending,
    Descending
};

struct ChangeHotkeyResult
{
    size_t previous_owner_index;
    int prev_hotkey_id = -1;
};

struct MediaLocation
{
    uint64_t list_id;
    size_t file_index;
};

struct MediaIndexes
{
    size_t list_index;
    size_t file_index;
};