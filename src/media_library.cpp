#include "media_library.h"

#include <algorithm>
#include <cassert>

void MediaLibrary::AddFile(const MediaInfo &file)
{
    if (!file.hotkey.IsEmpty())
    {
        size_t index = media_files_.size();
        AddHotkeyData(file, index);
    }

    media_files_.push_back(file);
}

void MediaLibrary::AddFile(MediaInfo &&file)
{
    if (!file.hotkey.IsEmpty())
    {
        size_t index = media_files_.size();
        AddHotkeyData(file, index);
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
    if (!media.hotkey.IsEmpty())
    {
        DeleteHotkeyData(media);
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

/*Возвращает позицию предыдущего владельца горячей клавиши и id горячей клавиши*/
std::optional<ChangeHotkeyResult> MediaLibrary::ChangeHotkey(size_t index, const Hotkey &hotkey)
{
    std::optional<ChangeHotkeyResult> result;
    if (index >= media_files_.size())
    {
        return std::nullopt;
    }

    MediaInfo &info = media_files_[index];
    if (!info.hotkey.IsEmpty())
    {
        DeleteHotkeyData(info);
        info.hotkey = {};
    }

    if (!hotkey.IsEmpty())
    {
        auto it = id_by_hotkeys_.find(hotkey);
        if (it != id_by_hotkeys_.end())
        {
            uint64_t previous_file_id = it->second;
            auto it_index = index_by_id_with_hotkey_.find(previous_file_id);
            assert(it_index != index_by_id_with_hotkey_.end());

            size_t previous_owner_index = it_index->second;
            MediaInfo &prev_owner = media_files_.at(previous_owner_index);

            ChangeHotkeyResult struct_res{previous_owner_index, DeleteHotkeyData(prev_owner)};
            result = struct_res;

            prev_owner.hotkey = {};
        }

        info.hotkey = hotkey;
        AddHotkeyData(info, index);
    }

    return result;
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

std::optional<size_t> MediaLibrary::GetMediaFileIndexByHotkey(const Hotkey &hotkey) const
{
    std::optional<size_t> res;

    auto it = id_by_hotkeys_.find(hotkey);
    if (it == id_by_hotkeys_.end())
    {
        return std::nullopt;
    }

    res = index_by_id_with_hotkey_.at(it->second);
    return res;
}

std::vector<std::pair<Hotkey, int>> MediaLibrary::GetGlobalHotkeys() const
{
    std::vector<std::pair<Hotkey, int>> res;
    for (const auto &[hotkey_id, media_id] : media_id_by_hotkey_id_)
    {
        auto it = index_by_id_with_hotkey_.find(media_id);
        assert(it != index_by_id_with_hotkey_.end());

        size_t index = it->second;
        assert(index < media_files_.size());

        res.push_back({media_files_.at(index).hotkey, hotkey_id});
    }

    return res;
}

int MediaLibrary::GetGlobalHotkeyIdByFileId(uint64_t file_id) const
{
    auto hotkey_id_it = hotkey_id_by_media_id_.find(file_id);
    assert(hotkey_id_it != hotkey_id_by_media_id_.end());

    return hotkey_id_it->second;
}

size_t MediaLibrary::GetMediaFileIndexByGlobalHotkeyId(int hotkey_id) const
{
    auto it_media_id = media_id_by_hotkey_id_.find(hotkey_id);
    assert(it_media_id != media_id_by_hotkey_id_.end());

    uint64_t media_id = it_media_id->second;

    auto it_index = index_by_id_with_hotkey_.find(media_id);
    assert(it_index != index_by_id_with_hotkey_.end());

    return it_index->second;
}

int MediaLibrary::GetLastHotkeyId() const
{
    return current_hotkey_id_;
}

void MediaLibrary::UpdateIndexMap()
{
    index_by_id_with_hotkey_.clear();

    for (size_t i = 0; i < media_files_.size(); ++i)
    {
        const MediaInfo &info = media_files_[i];
        if (!info.hotkey.IsEmpty())
        {
            index_by_id_with_hotkey_[info.id] = i;
        }
    }
}

void MediaLibrary::AddHotkeyData(const MediaInfo &file, size_t index)
{
    id_by_hotkeys_[file.hotkey] = file.id;
    index_by_id_with_hotkey_[file.id] = index;

    ++current_hotkey_id_;
    media_id_by_hotkey_id_[current_hotkey_id_] = file.id;
    hotkey_id_by_media_id_[file.id] = current_hotkey_id_;
}

/*Возвращает id удаляемой горячей клавиши*/
int MediaLibrary::DeleteHotkeyData(const MediaInfo &file)
{
    id_by_hotkeys_.erase(file.hotkey);
    index_by_id_with_hotkey_.erase(file.id);

    auto hotkey_id_it = hotkey_id_by_media_id_.find(file.id);
    assert(hotkey_id_it != hotkey_id_by_media_id_.end());

    int hotkey_id = hotkey_id_it->second;
    media_id_by_hotkey_id_.erase(hotkey_id);
    hotkey_id_by_media_id_.erase(file.id);

    return hotkey_id;
}
