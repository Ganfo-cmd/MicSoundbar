#pragma once
#include "interface_media_handler.h"

#include <filesystem>

const std::filesystem::path audio_folder_const = "D:\\audio"; /*заглушка*/

class MediaFileHandler : public InterfaceMediaFileHandler
{
public:
    MediaFileHandler();
    ~MediaFileHandler() = default;

    std::vector<MediaInfo> GetMediaFilesInfo() const override;
    bool IsAvailableFile(const std::string &file_path) const override;

private:
    std::filesystem::path audio_folder_ = audio_folder_const;
    std::vector<MediaInfo> media_files_;

    double GetMediaFileDuration(const std::string &file_path) const;
};