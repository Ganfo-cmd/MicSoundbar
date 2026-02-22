#pragma once
#include "media_file_info.h"

#include <vector>
#include <optional>
#include <filesystem>

class MediaScanner
{
public:
    MediaScanner() = default;

    std::vector<MediaInfo> ScanFolder(const std::filesystem::path &folder_path, uint64_t &next_id) const;
    std::vector<MediaInfo> ScanListFiles(const std::vector<std::filesystem::path> &files, uint64_t &next_id) const;
    std::optional<MediaInfo> ScanFile(const std::filesystem::path &file_path, uint64_t &next_id) const;

private:
    uint32_t GetMediaFileDuration(const std::string &file_path) const;
};