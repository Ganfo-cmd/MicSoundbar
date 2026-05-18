#pragma once
#include "media_file_info.h"

#include <vector>
#include <optional>
#include <unordered_map>

class MediaLibrary
{
public:
    MediaLibrary() = default;
    ~MediaLibrary() = default;

    void AddFile(const MediaInfo &file);
    void AddFile(MediaInfo &&file);
    void DeleteFile(size_t index);
    void RenameFile(size_t index, std::string new_name);
    void MoveFile(size_t from, size_t to);
    void Sort(SortField field, SortOrder order);
    void UpdateAvailability(bool is_available, size_t row);
    std::optional<size_t> ChangeHotkey(size_t index, const Hotkey &hotkey);

    size_t Size() const;

    const MediaInfo &GetMediaFileInfo(size_t index) const;
    const std::vector<MediaInfo> &GetAllMediaInfo() const;
    std::optional<size_t> GetMediaFileIndexByHotkey(const Hotkey &hotkey) const;

private:
    struct HotkeyHash
    {
        size_t operator()(const Hotkey &hk) const
        {
            size_t h1 = std::hash<uint32_t>()(hk.scan_code);
            size_t h2 = std::hash<uint32_t>()(hk.modifiers);

            return h1 ^ (h2 << 7);
        }
    };

    std::vector<MediaInfo> media_files_;
    std::unordered_map<uint64_t, size_t> index_by_id_with_hotkey_;
    std::unordered_map<Hotkey, uint64_t, HotkeyHash> id_by_hotkeys_;

    void UpdateIndexMap();
};