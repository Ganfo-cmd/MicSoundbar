#pragma once
#include "media_file_info.h"

#include <vector>
#include <string>
#include <filesystem>

class InterfaceMediaFileHandler
{
public:
    virtual ~InterfaceMediaFileHandler() = default;

    virtual void MoveFile(size_t from, size_t to) = 0;
    virtual void Sort(SortField field, SortOrder order) = 0;
    virtual bool UpdateAvailability(size_t row) = 0;
    virtual void DeleteFile(size_t index) = 0;
    virtual void SaveData() = 0;

    virtual void AddFilesInLibrary(const std::vector<std::filesystem::path> &files) = 0;

    virtual size_t Size() const = 0;
    virtual const MediaInfo &GetMediaFileInfo(size_t index) const = 0;
    virtual const std::vector<MediaInfo> &GetAllMediaInfo() const = 0;
};