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
    std::optional<ChangeHotkeyResult> ChangeHotkey(size_t index, const Hotkey &hotkey);

    size_t Size() const;

    const MediaInfo &GetMediaFileInfo(size_t index) const;
    const std::vector<MediaInfo> &GetAllMediaInfo() const;
    std::optional<size_t> GetMediaFileIndexByHotkey(const Hotkey &hotkey) const;

    std::vector<std::pair<Hotkey, int>> GetGlobalHotkeys() const;
    int GetGlobalHotkeyIdByFileId(uint64_t file_id) const;
    size_t GetMediaFileIndexByGlobalHotkeyId(int hotkey_id) const;
    int GetLastHotkeyId() const;

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

    int current_hotkey_id_ = 0;
    std::unordered_map<int, uint64_t> media_id_by_hotkey_id_;
    std::unordered_map<uint64_t, int> hotkey_id_by_media_id_;

    void UpdateIndexMap();
    void AddHotkeyData(const MediaInfo &file, size_t index);
    int DeleteHotkeyData(const MediaInfo &file);
};