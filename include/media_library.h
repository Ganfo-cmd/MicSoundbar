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

    void SetData(std::vector<MediaList> data);

    void RenameList(uint64_t list_id, std::string name);
    void AddList(uint64_t &next_list_id, std::string new_name);

    void AddFiles(uint64_t list_id, std::vector<MediaInfo> &&file_list);
    void DeleteFile(uint64_t list_id, size_t index);
    void RenameFile(uint64_t list_id, size_t index, std::string new_name);
    void MoveFile(uint64_t list_id, size_t from, size_t to);
    void Sort(uint64_t list_id, SortField field, SortOrder order);
    void UpdateAvailability(uint64_t list_id, bool is_available, size_t row);
    std::optional<ChangeHotkeyResult> ChangeHotkey(uint64_t list_id, size_t file_index, const Hotkey &hotkey);

    size_t GetMediaListSize(uint64_t list_id) const;
    const std::vector<MediaInfo> &GetMediaFiles(uint64_t list_id) const;
    const MediaInfo &GetMediaFileInfo(uint64_t list_id, size_t index) const;
    const std::vector<MediaList> &GetAllMediaLists() const;

    std::optional<MediaIndexes> GetMediaFileIndexesByHotkey(const Hotkey &hotkey) const;
    std::vector<std::pair<Hotkey, int>> GetGlobalHotkeys() const;
    int GetGlobalHotkeyIdByHotkey(const Hotkey &hotkey) const;
    const Hotkey &GetHotkeyByHotkeyId(int hotkey_id) const;
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

    std::vector<MediaList> media_lists_;
    std::unordered_map<uint64_t, size_t> list_index_by_list_id_;

    std::unordered_map<Hotkey, MediaLocation, HotkeyHash> media_location_by_hotkey_;

    int last_hotkey_id_ = 0;
    std::unordered_map<int, Hotkey> id_to_hotkey_;
    std::unordered_map<Hotkey, int, HotkeyHash> hotkey_to_id_;

    std::vector<MediaInfo> &GetMediaFiles(uint64_t list_id);

    void UpdateHotkeyLocation(uint64_t list_id);
    void AddHotkeyData(uint64_t list_id, size_t file_index, const Hotkey &hotkey);
    int DeleteHotkeyData(const Hotkey &hotkey);
};