#pragma once
#include "media_file_info.h"

#include <filesystem>
#include <vector>
#include <string>

const std::filesystem::path audio_folder_const = "D:\\audio"; /*заглушка*/

class MediaFileHandler
{
public:
    MediaFileHandler();
    ~MediaFileHandler() = default;

    std::vector<MediaInfo> GetMediaFilesInfo() const;

private:
    std::filesystem::path audio_folder_ = audio_folder_const;
    std::vector<MediaInfo> media_files_;

    double GetMediaFileDuration(const std::string &file_path) const;
};