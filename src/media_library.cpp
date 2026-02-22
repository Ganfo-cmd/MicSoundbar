#include "media_library.h"

#include <algorithm>

void MediaLibrary::AddFile(const MediaInfo &file)
{
    media_files_.push_back(file);
}

void MediaLibrary::AddFile(MediaInfo &&file)
{
    media_files_.push_back(std::move(file));
}

void MediaLibrary::RemoveFile(size_t index)
{
    if (index >= media_files_.size())
    {
        return;
    }

    media_files_.erase(media_files_.begin() + index);
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

    MediaInfo file = std::move(media_files_[from]);
    media_files_.erase(media_files_.begin() + from);
    media_files_.insert(media_files_.begin() + to, file);
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
}

void MediaLibrary::UpdateAvailability(bool is_available, size_t row)
{
    if (row >= media_files_.size())
    {
        return;
    }
    media_files_[row].available = is_available;
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
