#include "media_files.h"
#include "mp3_decoder.h"

MediaFileHandler::MediaFileHandler() : media_json_("D:\\audio\\library.json")
{
    bool success;
    const auto list = media_json_.Load(next_id_, success);

    if (!success)
    {
        for (const auto &file : std::filesystem::directory_iterator(audio_folder_))
        {
            const auto file_path = file.path();
            if (file_path.extension() == ".mp3")
            {
                const std::string file_path_str = file_path.string();

                MediaInfo media_info;
                media_info.id = next_id_++;
                media_info.path = file_path_str;
                media_info.name = file_path.filename().string();
                media_info.duration = GetMediaFileDuration(file_path_str);
                media_library_.AddFile(media_info);
            }
        }
        media_json_.Save(media_library_.GetAllMediaInfo(), next_id_);
    }
    else
    {
        for (const auto &elem : list)
        {
            media_library_.AddFile(elem);
        }
    }
}

void MediaFileHandler::MoveFile(size_t from, size_t to)
{
    media_library_.MoveFile(from, to);
}

void MediaFileHandler::Sort(SortField field, SortOrder order)
{
    media_library_.Sort(field, order);
}

size_t MediaFileHandler::Size() const
{
    return media_library_.Size();
}

const MediaInfo &MediaFileHandler::GetMediaFileInfo(size_t index) const
{
    return media_library_.GetMediaFileInfo(index);
}

const std::vector<MediaInfo> &MediaFileHandler::GetAllMediaInfo() const
{
    return media_library_.GetAllMediaInfo();
}

bool MediaFileHandler::UpdateAvailability(size_t row)
{
    const MediaInfo &file = media_library_.GetMediaFileInfo(row);
    bool exists = std::filesystem::exists(file.path);
    media_library_.UpdateAvailability(exists, row);
    return exists;
}

uint32_t MediaFileHandler::GetMediaFileDuration(const std::string &file_path) const
{
    return MP3Decoder::GetDuration(file_path);
}