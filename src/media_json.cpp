#include "media_json.h"

MediaJSON::MediaJSON(const std::string &file_path) : file_path_(file_path)
{
}

std::string MediaJSON::EscapeJsonString(const std::string &path) const
{
    std::string result;
    result.reserve(path.size());

    for (char ch : path)
    {
        if (ch == '\\')
        {
            result += "\\\\";
        }
        else if (ch == '"')
        {
            result += "\\\"";
        }
        else
        {
            result += ch;
        }
    }

    return result;
}

bool MediaJSON::Save(const std::vector<MediaList> &media_lists, uint64_t next_list_id, uint64_t next_file_id) const
{
    std::filesystem::path temp_path = file_path_;
    temp_path += ".tmp";

    {
        std::ofstream file(temp_path, std::ios::trunc);
        if (!file)
        {
            return false;
        }

        file << "{\n";
        file << "  \"next_list_id\": " << next_list_id << ",\n";
        file << "  \"next_file_id\": " << next_file_id << ",\n";
        file << "  \"lists\": [\n";
        for (size_t i = 0; i < media_lists.size(); ++i)
        {
            const MediaList &media_list = media_lists[i];
            file << "    {\n";
            file << "      \"id\": " << media_list.id << ",\n";
            file << "      \"name\": \"" << EscapeJsonString(media_list.name) << "\",\n";
            file << "      \"media\": [\n";

            WriteMedia(file, media_list.media);

            file << "      ]\n";
            file << "    }";

            if (i + 1 < media_lists.size())
            {
                file << ",";
            }
            file << "\n";
        }

        file << "  ]\n}\n";

        if (!file)
        {
            return false;
        }
    }

    std::error_code error;
    std::filesystem::rename(temp_path, file_path_, error);

    if (error)
    {
        std::filesystem::remove(temp_path);
        return false;
    }

    return true;
}

void MediaJSON::WriteMedia(std::ofstream &file, const std::vector<MediaInfo> &media) const
{
    for (size_t i = 0; i < media.size(); ++i)
    {
        const MediaInfo &media_info = media[i];
        file << "        {\n";
        file << "          \"id\": " << media_info.id << ",\n";
        file << "          \"name\": \"" << EscapeJsonString(media_info.name) << "\",\n";
        file << "          \"path\": \"" << EscapeJsonString(media_info.path) << "\",\n";
        file << "          \"duration\": " << media_info.duration << ",\n";
        file << "          \"hotkey_scan_code\": " << media_info.hotkey.scan_code << ",\n";
        file << "          \"hotkey_modifiers\": " << media_info.hotkey.modifiers << ",\n";
        file << "          \"hotkey_display\": \"" << EscapeJsonString(media_info.hotkey.display) << "\"\n";
        file << "        }";

        if (i + 1 < media.size())
        {
            file << ",";
        }
        file << "\n";
    }
}

std::vector<MediaList> MediaJSON::Load(uint64_t &next_list_id, uint64_t &next_file_id, bool &success) const
{
    std::ifstream file(file_path_);
    if (!file)
    {
        success = false;
        return {};
    }

    char ch;
    if (!(file >> ch) || ch != '{')
    {
        success = false;
        return {};
    }

    std::vector<MediaList> result;
    for (; file >> ch && ch != '}';)
    {
        if (ch == '"')
        {
            std::string key = LoadString(file);
            if (!(file >> ch) || ch != ':')
            {
                success = false;
                return {};
            }

            if (key == "next_list_id")
            {
                next_list_id = LoadId(file);
            }
            else if (key == "next_file_id")
            {
                next_file_id = LoadId(file);
            }
            else if (key == "lists")
            {
                if (!(file >> ch) || ch != '[')
                {
                    success = false;
                    return {};
                }

                for (char c; file >> c && c != ']';)
                {
                    if (c == '{')
                    {
                        result.emplace_back(LoadMediaList(file));
                    }
                }
            }
        }
    }
    success = true;
    return result;
}

MediaList MediaJSON::LoadMediaList(std::ifstream &file) const
{
    MediaList list;
    for (char ch; file >> ch && ch != '}';)
    {
        if (ch == '"')
        {
            std::string key = LoadString(file);
            if (!(file >> ch) || ch != ':')
            {
                return {};
            }

            if (key == "id")
            {
                list.id = LoadId(file);
            }
            else if (key == "name")
            {
                file >> std::ws;
                if (!file.get(ch) || ch != '"')
                    return {};
                list.name = LoadString(file);
            }
            else if (key == "media")
            {
                if (!(file >> ch) || ch != '[')
                {
                    return {};
                }

                auto &media = list.media;
                for (char c; file >> c && c != ']';)
                {
                    if (c == '{')
                    {
                        media.emplace_back(LoadMediaInfo(file));
                    }
                }
            }
        }
    }

    return list;
}

MediaInfo MediaJSON::LoadMediaInfo(std::ifstream &file) const
{
    MediaInfo media_info;
    for (char ch; file >> ch && ch != '}';)
    {
        if (ch == '"')
        {
            std::string key = LoadString(file);
            if (file >> ch && ch != ':')
            {
                return {};
            }

            if (key == "id")
            {
                media_info.id = LoadId(file);
            }
            else if (key == "name")
            {
                file >> std::ws;
                if (!file.get(ch) || ch != '"')
                    return {};
                media_info.name = LoadString(file);
            }
            else if (key == "path")
            {
                file >> std::ws;
                if (!file.get(ch) || ch != '"')
                    return {};
                media_info.path = LoadString(file);
            }
            else if (key == "duration")
            {
                media_info.duration = LoadDuration(file);
            }
            else if (key == "hotkey_scan_code")
            {
                media_info.hotkey.scan_code = LoadDuration(file);
            }
            else if (key == "hotkey_modifiers")
            {
                media_info.hotkey.modifiers = LoadDuration(file);
            }
            else if (key == "hotkey_display")
            {
                file >> std::ws;
                if (!file.get(ch) || ch != '"')
                    return {};
                media_info.hotkey.display = LoadString(file);
            }
        }
    }
    return media_info;
}

std::string MediaJSON::LoadString(std::ifstream &file) const
{
    std::string result;
    char ch;

    while (file.get(ch))
    {
        if (ch == '\\')
        {
            char next_ch;
            if (file.get(next_ch))
            {
                result.push_back(next_ch);
            }
        }
        else if (ch == '"')
        {
            break;
        }
        else
        {
            result.push_back(ch);
        }
    }

    return result;
}

uint64_t MediaJSON::LoadId(std::ifstream &file) const
{
    char ch;
    std::string val;

    file >> std::ws;

    while (file.get(ch))
    {
        if (isdigit(ch))
        {
            val.push_back(ch);
        }
        else if (ch == ',' || ch == '}')
        {
            file.putback(ch);
            break;
        }
    }

    try
    {
        return std::stoull(val);
    }
    catch (...)
    {
        return 0;
    }
}

uint32_t MediaJSON::LoadDuration(std::ifstream &file) const
{
    char ch;
    std::string val;

    file >> std::ws;

    while (file.get(ch))
    {
        if (isdigit(ch))
        {
            val.push_back(ch);
        }
        else if (ch == ',' || ch == '}')
        {
            file.putback(ch);
            break;
        }
    }

    try
    {
        return std::stoul(val);
    }
    catch (...)
    {
        return 0;
    }
}
