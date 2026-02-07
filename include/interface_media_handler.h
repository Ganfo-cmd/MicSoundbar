#pragma once
#include "media_file_info.h"

#include <vector>
#include <string>

class InterfaceMediaFileHandler
{
public:
    virtual std::vector<MediaInfo> GetMediaFilesInfo() const = 0;
    virtual bool IsAvailableFile(const std::string &file_path) const = 0;
};