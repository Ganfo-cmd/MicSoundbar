#include "media_files.h"

MediaFileHandler::MediaFileHandler() : media_json_("D:\\audio\\library.json")
{
    Initialize();
}

void MediaFileHandler::Initialize()
{
    bool success;
    auto data = media_json_.Load(next_list_id_, next_file_id_, success);

    if (!success)
    {
        data = media_scanner_.ScanFolder(audio_folder_, next_list_id_, next_file_id_);
        media_json_.Save(data, next_list_id_, next_file_id_);
    }

    media_library_.SetData(std::move(data));
}

/*Возвращает позицию предыдущего владельца горячей клавиши и id горячей клавиши*/
std::optional<ChangeHotkeyResult> MediaFileHandler::ChangeHotkey(uint64_t list_id, size_t file_index, const Hotkey &hotkey)
{
    return media_library_.ChangeHotkey(list_id, file_index, hotkey);
}

void MediaFileHandler::RenameFile(uint64_t list_id, size_t file_index, std::string new_name)
{
    media_library_.RenameFile(list_id, file_index, new_name);
}

void MediaFileHandler::MoveFile(uint64_t list_id, size_t from, size_t to)
{
    media_library_.MoveFile(list_id, from, to);
}

void MediaFileHandler::Sort(uint64_t list_id, SortField field, SortOrder order)
{
    media_library_.Sort(list_id, field, order);
}

bool MediaFileHandler::UpdateAvailability(uint64_t list_id, size_t row)
{
    const MediaInfo &file = media_library_.GetMediaFileInfo(list_id, row);
    bool exists = std::filesystem::exists(file.path);
    media_library_.UpdateAvailability(list_id, exists, row);
    return exists;
}

void MediaFileHandler::DeleteFile(uint64_t list_id, size_t file_index)
{
    media_library_.DeleteFile(list_id, file_index);
}

void MediaFileHandler::SaveData()
{
    media_json_.Save(media_library_.GetAllMediaLists(), next_list_id_, next_file_id_);
}

uint64_t MediaFileHandler::AddList(std::string name)
{
    media_library_.AddList(next_list_id_, name);
    return next_list_id_ - 1;
}

std::vector<int> MediaFileHandler::DeleteList(int list_index)
{
    return media_library_.DeleteList(list_index);
}

void MediaFileHandler::RenameList(uint64_t list_id, std::string name)
{
    media_library_.RenameList(list_id, name);
}

void MediaFileHandler::AddFilesInLibrary(uint64_t list_id, const std::vector<std::filesystem::path> &files)
{
    std::vector<MediaInfo> media_info_list = media_scanner_.ScanListFiles(files, next_file_id_);

    media_library_.AddFiles(list_id, std::move(media_info_list));

    media_json_.Save(media_library_.GetAllMediaLists(), next_list_id_, next_file_id_);
}

size_t MediaFileHandler::GetMediaListSize(uint64_t media_list_id) const
{
    return media_library_.GetMediaListSize(media_list_id);
}

const std::vector<MediaInfo> &MediaFileHandler::GetMediaFiles(uint64_t list_id) const
{
    return media_library_.GetMediaFiles(list_id);
}

const MediaInfo &MediaFileHandler::GetMediaFileInfo(uint64_t list_id, size_t file_index) const
{
    return media_library_.GetMediaFileInfo(list_id, file_index);
}

const std::vector<MediaList> &MediaFileHandler::GetAllMediaLists() const
{
    return media_library_.GetAllMediaLists();
}

std::optional<MediaIndexes> MediaFileHandler::GetMediaFileIndexesByHotkey(const Hotkey &hotkey) const
{
    return media_library_.GetMediaFileIndexesByHotkey(hotkey);
}

std::vector<std::pair<Hotkey, int>> MediaFileHandler::GetGlobalHotkeys() const
{
    return media_library_.GetGlobalHotkeys();
}

int MediaFileHandler::GetGlobalHotkeyIdByHotkey(const Hotkey &hotkey) const
{
    return media_library_.GetGlobalHotkeyIdByHotkey(hotkey);
}

const Hotkey &MediaFileHandler::GetHotkeyByHotkeyId(int hotkey_id) const
{
    return media_library_.GetHotkeyByHotkeyId(hotkey_id);
}

int MediaFileHandler::GetLastHotkeyId() const
{
    return media_library_.GetLastHotkeyId();
}