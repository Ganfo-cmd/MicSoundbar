#include "media_json.h"

MediaJSON::MediaJSON(const std::string &file_path) : file_path_(file_path)
{
}

std::string MediaJSON::EscapePath(const std::string &path) const
{
    std::string result;
    result.reserve(path.size());

    for (char ch : path)
    {
        if (ch == '\\')
        {
            result += "\\\\";
        }
        else
        {
            result += ch;
        }
    }

    return result;
}

bool MediaJSON::Save(const std::vector<MediaInfo> &media_list, uint64_t next_id) const
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
        file << "  \"next_id\": " << next_id << ",\n";
        file << "  \"media\": [\n";

        for (size_t i = 0; i < media_list.size(); ++i)
        {
            const MediaInfo &media = media_list[i];
            file << "    {\n";
            file << "      \"id\": " << media.id << ",\n";
            file << "      \"name\": \"" << media.name << "\",\n";
            file << "      \"path\": \"" << EscapePath(media.path) << "\",\n";
            file << "      \"duration\": " << media.duration << "\n";
            file << "    }";

            if (i + 1 < media_list.size())
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

std::vector<MediaInfo> MediaJSON::Load(uint64_t &next_id, bool &success) const
{
    std::ifstream file(file_path_);
    if (!file)
    {
        next_id = 1;
        success = false;
        return {};
    }

    char ch;
    if (!(file >> ch) || ch != '{')
    {
        next_id = 1;
        success = false;
        return {};
    }

    std::vector<MediaInfo> result;
    for (; file >> ch && ch != '}';)
    {
        if (ch == '"')
        {
            std::string key = LoadString(file);
            if (!(file >> ch) && ch != ':')
            {
                next_id = 1;
                success = false;
                return {};
            }

            if (key == "next_id")
            {
                next_id = LoadId(file);
            }
            else if (key == "media")
            {
                if (file >> ch && ch != '[')
                {
                    next_id = 1;
                    success = false;
                    return {};
                }

                for (char c; file >> c && c != ']';)
                {
                    if (c == '{')
                    {
                        result.push_back(LoadMediaInfo(file));
                    }
                }
            }
        }
    }

    success = true;
    return result;
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
