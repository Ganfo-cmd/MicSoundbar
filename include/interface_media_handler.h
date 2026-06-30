#pragma once
#include "media_file_info.h"

#include <vector>
#include <string>
#include <filesystem>
#include <optional>

class InterfaceMediaFileHandler
{
public:
    virtual ~InterfaceMediaFileHandler() = default;

    /*Возвращает позицию предыдущего владельца горячей клавиши и id горячей клавиши*/
    virtual std::optional<ChangeHotkeyResult> ChangeHotkey(uint64_t list_id, size_t file_index, const Hotkey &hotkey) = 0;
    virtual void RenameFile(uint64_t list_id, size_t file_index, std::string new_name) = 0;
    virtual void MoveFile(uint64_t list_id, size_t from, size_t to) = 0;
    virtual void Sort(uint64_t list_id, SortField field, SortOrder order) = 0;
    virtual bool UpdateAvailability(uint64_t list_id, size_t row) = 0;
    virtual void DeleteFile(uint64_t list_id, size_t file_index) = 0;
    virtual void SaveData() = 0;

    virtual uint64_t AddList(std::string name) = 0;
    virtual std::vector<int> DeleteList(int list_index) = 0;
    virtual void RenameList(uint64_t list_id, std::string name) = 0;
    virtual void AddFilesInLibrary(uint64_t list_id, const std::vector<std::filesystem::path> &files) = 0;

    virtual size_t GetMediaListSize(uint64_t list_id) const = 0;
    virtual const std::vector<MediaInfo> &GetMediaFiles(uint64_t id) const = 0;
    virtual const MediaInfo &GetMediaFileInfo(uint64_t list_id, size_t file_index) const = 0;
    virtual const std::vector<MediaList> &GetAllMediaLists() const = 0;

    virtual std::optional<MediaIndexes> GetMediaFileIndexesByHotkey(const Hotkey &hotkey) const = 0;
    virtual std::vector<std::pair<Hotkey, int>> GetGlobalHotkeys() const = 0;
    virtual int GetGlobalHotkeyIdByHotkey(const Hotkey &hotkey) const = 0;
    virtual const Hotkey &GetHotkeyByHotkeyId(int hotkey_id) const = 0;
    virtual int GetLastHotkeyId() const = 0;
};