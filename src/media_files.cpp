#include "media_files.h"
#include "mp3_decoder.h"

MediaFileHandler::MediaFileHandler()
{
    for (const auto &file : std::filesystem::directory_iterator(audio_folder_))
    {
        const auto file_path = file.path();
        if (file_path.extension() == ".mp3")
        {
            const std::string file_path_str = file_path.string();

            MediaInfo media_info;
            media_info.path = file_path_str;
            media_info.name = file_path.filename().string();
            media_info.duration = GetMediaFileDuration(file_path_str);
            media_files_.push_back(media_info);
        }
    }
}

std::vector<MediaInfo> MediaFileHandler::GetMediaFilesInfo() const
{
    return media_files_;
}

bool MediaFileHandler::IsAvailableFile(const std::string &file_path) const
{
    return std::filesystem::exists(file_path);
}

double MediaFileHandler::GetMediaFileDuration(const std::string &file_path) const
{
    return MP3Decoder::GetDuration(file_path);
}