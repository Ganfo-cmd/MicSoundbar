#include "media_files.h"
#include <iostream>
MediaFileHandler::MediaFileHandler() : media_json_("D:\\audio\\library.json")
{
    Initialize();
}

void MediaFileHandler::Initialize()
{
    bool success;
    auto list = media_json_.Load(next_id_, success);

    if (!success)
    {
        list = media_scanner_.ScanFolder(audio_folder_, next_id_);
        media_json_.Save(list, next_id_);
    }

    for (auto &file : list)
    {
        media_library_.AddFile(std::move(file));
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