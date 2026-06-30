#include "media_library.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>

void MediaLibrary::SetData(std::vector<MediaList> data)
{
    media_lists_ = std::move(data);

    list_index_by_list_id_.clear();
    media_location_by_hotkey_.clear();

    last_hotkey_id_ = 0;
    id_to_hotkey_.clear();
    hotkey_to_id_.clear();

    for (size_t list_index = 0; list_index < media_lists_.size(); ++list_index)
    {
        const MediaList &list = media_lists_[list_index];
        list_index_by_list_id_[list.id] = list_index;

        const std::vector<MediaInfo> &media = list.media;
        for (size_t file_index = 0; file_index < media.size(); ++file_index)
        {
            const Hotkey &hotkey = media[file_index].hotkey;
            if (!hotkey.IsEmpty())
            {
                AddHotkeyData(list.id, file_index, hotkey);
            }
        }
    }
}

void MediaLibrary::RenameList(uint64_t list_id, std::string new_name)
{
    size_t index = list_index_by_list_id_.at(list_id);
    media_lists_[index].name = std::move(new_name);
}

void MediaLibrary::AddList(uint64_t &next_list_id, std::string name)
{
    MediaList list;
    list.id = next_list_id++;
    list.name = std::move(name);

    list_index_by_list_id_[list.id] = media_lists_.size();
    media_lists_.push_back(std::move(list));
}

void MediaLibrary::AddFiles(uint64_t list_id, std::vector<MediaInfo> &&file_list)
{
    std::vector<MediaInfo> &media = GetMediaFiles(list_id);

    for (MediaInfo &media_info : file_list)
    {
        media.push_back(std::move(media_info));
    }
}

void MediaLibrary::DeleteFile(uint64_t list_id, size_t index)
{
    std::vector<MediaInfo> &media = GetMediaFiles(list_id);
    assert(index < media.size());

    MediaInfo &media_info = media.at(index);
    if (!media_info.hotkey.IsEmpty())
    {
        media_location_by_hotkey_.erase(media_info.hotkey);
    }

    for (auto &[_, location] : media_location_by_hotkey_)
    {
        if (list_id == location.list_id && location.file_index > index)
        {
            --location.file_index;
        }
    }

    media.erase(media.begin() + index);
}

void MediaLibrary::RenameFile(uint64_t list_id, size_t index, std::string new_name)
{
    std::vector<MediaInfo> &media = GetMediaFiles(list_id);
    assert(index < media.size());

    media[index].name = std::move(new_name);
}

void MediaLibrary::MoveFile(uint64_t list_id, size_t from, size_t to)
{
    std::vector<MediaInfo> &media = GetMediaFiles(list_id);
    assert(from < media.size() && to < media.size());

    if (from == to)
    {
        return;
    }

    auto first = media.begin();
    if (from < to)
    {
        std::rotate(first + from, first + from + 1, first + to + 1);
    }
    else
    {
        std::rotate(first + to, first + from, first + from + 1);
    }

    UpdateHotkeyLocation(list_id);
}

void MediaLibrary::Sort(uint64_t list_id, SortField field, SortOrder order)
{
    std::vector<MediaInfo> &media = GetMediaFiles(list_id);

    std::sort(media.begin(), media.end(),
              [field, order](const MediaInfo &left, const MediaInfo &right)
              {
                  switch (field)
                  {
                  case SortField::Name:
                  {
                      return (order == SortOrder::Ascending)
                                 ? left.name < right.name
                                 : left.name > right.name;
                  }
                  case SortField::Duration:
                  {
                      return (order == SortOrder::Ascending)
                                 ? left.duration < right.duration
                                 : left.duration > right.duration;
                  }
                  case SortField::Hotkey:
                  {
                      bool left_empty = left.hotkey.IsEmpty();
                      bool right_empty = right.hotkey.IsEmpty();

                      if (left_empty != right_empty)
                          return !left_empty;

                      uint32_t left_scan_code = left.hotkey.scan_code;
                      uint32_t right_scan_code = right.hotkey.scan_code;

                      if (left_scan_code != right_scan_code)
                      {
                          return (order == SortOrder::Ascending)
                                     ? left_scan_code < right_scan_code
                                     : left_scan_code > right_scan_code;
                      }

                      return (order == SortOrder::Ascending)
                                 ? left.hotkey.modifiers < right.hotkey.modifiers
                                 : left.hotkey.modifiers > right.hotkey.modifiers;
                  }
                  }

                  return false;
              });

    UpdateHotkeyLocation(list_id);
}

void MediaLibrary::UpdateAvailability(uint64_t list_id, bool is_available, size_t row)
{
    std::vector<MediaInfo> &media = GetMediaFiles(list_id);
    assert(row < media.size());
    media[row].available = is_available;
}

/*Возвращает позицию предыдущего владельца горячей клавиши и id горячей клавиши*/
std::optional<ChangeHotkeyResult> MediaLibrary::ChangeHotkey(uint64_t list_id, size_t file_index, const Hotkey &hotkey)
{
    std::optional<ChangeHotkeyResult> result;

    std::vector<MediaInfo> &media = GetMediaFiles(list_id);
    assert(file_index < media.size());

    MediaInfo &info = media[file_index];
    if (!info.hotkey.IsEmpty())
    {
        DeleteHotkeyData(info.hotkey);
    }

    if (!hotkey.IsEmpty())
    {
        auto it = media_location_by_hotkey_.find(hotkey);
        if (it != media_location_by_hotkey_.end())
        {
            size_t previous_owner_index = it->second.file_index;

            result = ChangeHotkeyResult{previous_owner_index, DeleteHotkeyData(hotkey)};
        }

        info.hotkey = hotkey;
        AddHotkeyData(list_id, file_index, hotkey);
    }

    return result;
}

const MediaInfo &MediaLibrary::GetMediaFileInfo(uint64_t list_id, size_t file_index) const
{
    const std::vector<MediaInfo> &media = GetMediaFiles(list_id);
    assert(file_index < media.size());
    return media[file_index];
}

const std::vector<MediaList> &MediaLibrary::GetAllMediaLists() const
{
    return media_lists_;
}

size_t MediaLibrary::GetMediaListSize(uint64_t list_id) const
{
    const std::vector<MediaInfo> &media = GetMediaFiles(list_id);
    return media.size();
}

const std::vector<MediaInfo> &MediaLibrary::GetMediaFiles(uint64_t list_id) const
{
    size_t index = list_index_by_list_id_.at(list_id);
    return media_lists_[index].media;
}

std::vector<MediaInfo> &MediaLibrary::GetMediaFiles(uint64_t list_id)
{
    return const_cast<std::vector<MediaInfo> &>(std::as_const(*this).GetMediaFiles(list_id));
}

std::optional<MediaIndexes> MediaLibrary::GetMediaFileIndexesByHotkey(const Hotkey &hotkey) const
{
    auto it = media_location_by_hotkey_.find(hotkey);
    if (it == media_location_by_hotkey_.end())
    {
        return std::nullopt;
    }

    const MediaLocation &location = it->second;
    size_t list_index = list_index_by_list_id_.at(location.list_id);
    return MediaIndexes{list_index, location.file_index};
}

std::vector<std::pair<Hotkey, int>> MediaLibrary::GetGlobalHotkeys() const
{
    std::vector<std::pair<Hotkey, int>> res;
    res.reserve(hotkey_to_id_.size());
    for (const auto &[hotkey, hotkey_id] : hotkey_to_id_)
    {
        res.push_back({hotkey, hotkey_id});
    }

    return res;
}

int MediaLibrary::GetGlobalHotkeyIdByHotkey(const Hotkey &hotkey) const
{
    return hotkey_to_id_.at(hotkey);
}

const Hotkey &MediaLibrary::GetHotkeyByHotkeyId(int hotkey_id) const
{
    return id_to_hotkey_.at(hotkey_id);
}

int MediaLibrary::GetLastHotkeyId() const
{
    return last_hotkey_id_;
}

void MediaLibrary::UpdateHotkeyLocation(uint64_t list_id)
{
    size_t list_index = list_index_by_list_id_.at(list_id);
    const std::vector<MediaInfo> &media = media_lists_[list_index].media;
    for (size_t file_index = 0; file_index < media.size(); ++file_index)
    {
        const Hotkey &hotkey = media[file_index].hotkey;
        if (!hotkey.IsEmpty())
        {
            media_location_by_hotkey_[hotkey] = {list_id, file_index};
        }
    }
}

void MediaLibrary::AddHotkeyData(uint64_t list_id, size_t file_index, const Hotkey &hotkey)
{
    media_location_by_hotkey_[hotkey] = {list_id, file_index};

    const int hotkey_id = ++last_hotkey_id_;
    id_to_hotkey_[hotkey_id] = hotkey;
    hotkey_to_id_[hotkey] = hotkey_id;
}

/*Возвращает id удаляемой горячей клавиши*/
int MediaLibrary::DeleteHotkeyData(const Hotkey &hotkey)
{
    auto it = media_location_by_hotkey_.find(hotkey);
    assert(it != media_location_by_hotkey_.end());

    MediaLocation location = it->second;
    media_location_by_hotkey_.erase(it);

    int hotkey_id = hotkey_to_id_[hotkey];
    hotkey_to_id_.erase(hotkey);
    id_to_hotkey_.erase(hotkey_id);

    size_t list_index = list_index_by_list_id_[location.list_id];
    media_lists_[list_index].media[location.file_index].hotkey = {};

    return hotkey_id;
}
