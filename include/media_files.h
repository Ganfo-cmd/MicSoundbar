#pragma once
#include "interface_media_handler.h"
#include "media_library.h"

#include <filesystem>

const std::filesystem::path audio_folder_const = "D:\\audio"; /*заглушка*/

class MediaFileHandler : public InterfaceMediaFileHandler
{
public:
    MediaFileHandler();
    ~MediaFileHandler() = default;

    void MoveFile(size_t from, size_t to) override;
    void Sort(SortField field, SortOrder order) override;
    bool UpdateAvailability(size_t row) override;

    size_t Size() const override;
    const MediaInfo &GetMediaFileInfo(size_t index) const override;
    const std::vector<MediaInfo> &GetAllMediaInfo() const override;

private:
    uint64_t next_id_ = 1; /*заглушка*/
    std::filesystem::path audio_folder_ = audio_folder_const;
    MediaLibrary media_library_;

    double GetMediaFileDuration(const std::string &file_path) const;
};