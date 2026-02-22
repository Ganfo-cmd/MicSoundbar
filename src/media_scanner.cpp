#include "media_scanner.h"
#include "mp3_decoder.h"

#include <algorithm>

std::vector<MediaInfo> MediaScanner::ScanFolder(const std::filesystem::path &folder_path, uint64_t &next_id) const
{
    if (!std::filesystem::exists(folder_path))
    {
        return {};
    }

    std::vector<MediaInfo> result;
    for (const auto &file : std::filesystem::directory_iterator(folder_path))
    {
        auto info = ScanFile(file.path(), next_id);
        if (info.has_value())
        {
            result.push_back(info.value());
        }
    }
    return result;
}

std::vector<MediaInfo> MediaScanner::ScanListFiles(const std::vector<std::filesystem::path> &files, uint64_t &next_id) const
{
    std::vector<MediaInfo> result;
    for (const auto &file_path : files)
    {
        auto info = ScanFile(file_path, next_id);
        if (info.has_value())
        {
            result.push_back(info.value());
        }
    }
    return result;
}

std::optional<MediaInfo> MediaScanner::ScanFile(const std::filesystem::path &file_path, uint64_t &next_id) const
{
    auto ext = file_path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext != ".mp3")
    {
        return std::nullopt;
    }

    MediaInfo media_info;
    const std::string file_path_str = file_path.string();

    media_info.id = next_id++;
    media_info.path = file_path_str;
    media_info.name = file_path.filename().string();
    media_info.duration = GetMediaFileDuration(file_path_str);

    return media_info;
}

uint32_t MediaScanner::GetMediaFileDuration(const std::string &file_path) const
{
    return MP3Decoder::GetDuration(file_path);
}