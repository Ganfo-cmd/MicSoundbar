#pragma once
#include "media_file_info.h"

#include <vector>

class MediaJSON
{
public:
    MediaJSON();
    std::vector<MediaInfo> LoadJSONFile();
    void SaveJSONFile();

private:
};