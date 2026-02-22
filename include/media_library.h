#pragma once
#include "media_file_info.h"

#include <vector>

class MediaLibrary
{
public:
    MediaLibrary() = default;
    ~MediaLibrary() = default;

    void AddFile(const MediaInfo &file);
    void AddFile(MediaInfo &&file);
    void RemoveFile(size_t index);
    void MoveFile(size_t from, size_t to);
    void Sort(SortField field, SortOrder order);
    void UpdateAvailability(bool is_available, size_t row);

    const MediaInfo &GetMediaFileInfo(size_t index) const;
    const std::vector<MediaInfo> &GetAllMediaInfo() const;
    size_t Size() const;

private:
    std::vector<MediaInfo> media_files_;
};