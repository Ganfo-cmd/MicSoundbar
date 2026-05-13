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
    std::optional<size_t> ChangeHotkey(size_t index, std::string hotkey);

    const MediaInfo &GetMediaFileInfo(size_t index) const;
    const std::vector<MediaInfo> &GetAllMediaInfo() const;
    size_t Size() const;
    size_t GetMediaFileIndexById(uint64_t id) const;

private:
    std::vector<MediaInfo> media_files_;
    std::unordered_map<uint64_t, size_t> index_by_id_with_hotkey_;
    std::unordered_map<std::string, uint64_t> id_by_hotkeys_;

    void UpdateIndexMap();
};