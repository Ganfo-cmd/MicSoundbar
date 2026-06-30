#pragma once
#include "media_file_info.h"

#include <vector>
#include <fstream>
#include <filesystem>

class MediaJSON
{
public:
    MediaJSON(const std::string &file_path);

    std::vector<MediaList> Load(uint64_t &next_list_id, uint64_t &next_file_id, bool &success) const;
    bool Save(const std::vector<MediaList> &media_lists, uint64_t next_list_id, uint64_t next_file_id) const;

private:
    std::filesystem::path file_path_;

    void WriteMedia(std::ofstream &file, const std::vector<MediaInfo> &media) const;

    MediaList LoadMediaList(std::ifstream &file) const;
    MediaInfo LoadMediaInfo(std::ifstream &file) const;
    std::string LoadString(std::ifstream &file) const;
    uint64_t LoadId(std::ifstream &file) const;
    uint32_t LoadDuration(std::ifstream &file) const;

    std::string EscapeJsonString(const std::string &path) const;
};