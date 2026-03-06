#pragma once
#include "media_file_info.h"

#include <vector>
#include <fstream>
#include <filesystem>

class MediaJSON
{
public:
    MediaJSON(const std::string &file_path);

    std::vector<MediaInfo> Load(uint64_t &next_id, bool &success) const;
    bool Save(const std::vector<MediaInfo> &media_list, uint64_t next_id) const;

private:
    std::filesystem::path file_path_;

    MediaInfo LoadMediaInfo(std::ifstream &file) const;
    std::string LoadString(std::ifstream &file) const;
    uint64_t LoadId(std::ifstream &file) const;
    uint32_t LoadDuration(std::ifstream &file) const;

    std::string EscapeJsonString(const std::string &path) const;
};