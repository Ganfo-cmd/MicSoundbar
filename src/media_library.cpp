#include "media_library.h"

#include <algorithm>

void MediaLibrary::AddFile(const MediaInfo &file)
{
    if (!file.hotkey.empty())
    {
        size_t index = media_files_.size();
        id_by_hotkeys_[file.hotkey] = file.id;
        index_by_id_with_hotkey_[file.id] = index;
    }

    media_files_.push_back(file);
}

void MediaLibrary::AddFile(MediaInfo &&file)
{
    if (!file.hotkey.empty())
    {
        size_t index = media_files_.size();
        id_by_hotkeys_[file.hotkey] = file.id;
        index_by_id_with_hotkey_[file.id] = index;
    }

    media_files_.push_back(std::move(file));
}

void MediaLibrary::DeleteFile(size_t index)
{
    if (index >= media_files_.size())
    {
        return;
    }

    MediaInfo &media = media_files_[index];
    if (!media.hotkey.empty())
    {
        id_by_hotkeys_.erase(media.hotkey);
        index_by_id_with_hotkey_.erase(media.id);
    }

    for (auto &[id, idx] : index_by_id_with_hotkey_)
    {
        if (idx > index)
        {
            --idx;
        }
    }

    media_files_.erase(media_files_.begin() + index);
}

void MediaLibrary::RenameFile(size_t index, std::string new_name)
{
    if (index >= media_files_.size())
    {
        return;
    }

    media_files_[index].name = std::move(new_name);
}

void MediaLibrary::MoveFile(size_t from, size_t to)
{
    if (from >= media_files_.size() || to >= media_files_.size())
    {
        return;
    }

    if (from == to)
    {
        return;
    }

    auto first = media_files_.begin();
    if (from < to)
    {
        std::rotate(first + from, first + from + 1, first + to + 1);
    }
    else
    {
        std::rotate(first + to, first + from, first + from + 1);
    }

    UpdateIndexMap();
}

void MediaLibrary::Sort(SortField field, SortOrder order)
{
    std::sort(media_files_.begin(), media_files_.end(),
              [field, order](const MediaInfo &left, const MediaInfo &right)
              {
                bool less = false;
                switch (field)
                {
                case SortField::Name:
                    less = left.name < right.name;
                    break;
                case SortField::Duration:
                    less = left.duration < right.duration;
                    break;
                }

                return order == SortOrder::Ascending ? less : !less; });

    UpdateIndexMap();
}

void MediaLibrary::UpdateAvailability(bool is_available, size_t row)
{
    if (row >= media_files_.size())
    {
        return;
    }
    media_files_[row].available = is_available;
}

/*Возвращает позицию предыдущего владельца горячей клавиши*/
std::optional<size_t> MediaLibrary::ChangeHotkey(size_t index, std::string hotkey)
{
    std::optional<size_t> previous_owner_index;
    if (index >= media_files_.size())
    {
        return previous_owner_index;
    }

    MediaInfo &info = media_files_[index];
    uint64_t id = info.id;
    if (!info.hotkey.empty())
    {
        id_by_hotkeys_.erase(info.hotkey);
        index_by_id_with_hotkey_.erase(id);
    }

    if (!hotkey.empty())
    {
        auto it = id_by_hotkeys_.find(hotkey);
        if (it != id_by_hotkeys_.end())
        {
            uint64_t previous_id = it->second;

            auto it_index = index_by_id_with_hotkey_.find(previous_id);
            if (it_index != index_by_id_with_hotkey_.end())
            {
                previous_owner_index = it_index->second;
                media_files_[it_index->second].hotkey.clear();
                index_by_id_with_hotkey_.erase(it_index);
            }

            id_by_hotkeys_.erase(it);
        }

        id_by_hotkeys_[hotkey] = id;
        index_by_id_with_hotkey_[id] = index;
    }

    info.hotkey = std::move(hotkey);
    return previous_owner_index;
}

const MediaInfo &MediaLibrary::GetMediaFileInfo(size_t index) const
{
    return media_files_.at(index);
}

const std::vector<MediaInfo> &MediaLibrary::GetAllMediaInfo() const
{
    return media_files_;
}

size_t MediaLibrary::Size() const
{
    return media_files_.size();
}

void MediaLibrary::UpdateIndexMap()
{
    for (size_t i = 0; i < media_files_.size(); ++i)
    {
        const MediaInfo &info = media_files_[i];
        if (!info.hotkey.empty())
        {
            index_by_id_with_hotkey_[info.id] = i;
        }
    }
}

size_t MediaLibrary::GetMediaFileIndexById(uint64_t id) const
{
    return index_by_id_with_hotkey_.at(id);
}
