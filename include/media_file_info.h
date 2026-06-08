#pragma once
#include <cstdint>
#include <string>

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