#pragma once
#include "interface_media_handler.h"
#include "media_library.h"
#include "media_json.h"
#include "media_scanner.h"

const std::filesystem::path audio_folder_const = "D:\\audio"; /*заглушка*/

class MediaFileHandler : public InterfaceMediaFileHandler
{
public:
    MediaFileHandler();
    ~MediaFileHandler() = default;

    void Initialize();

    std::optional<ChangeHotkeyResult> ChangeHotkey(uint64_t list_id, size_t file_index, const Hotkey &hotkey) override;
    void RenameFile(uint64_t list_id, size_t file_index, std::string new_name) override;
    void MoveFile(uint64_t list_id, size_t from, size_t to) override;
    void Sort(uint64_t list_id, SortField field, SortOrder order) override;
    bool UpdateAvailability(uint64_t list_id, size_t row) override;
    void DeleteFile(uint64_t list_id, size_t file_index) override;
    void SaveData() override;

    uint64_t AddList(std::string name) override;
    std::vector<int> DeleteList(int list_index) override;
    void RenameList(uint64_t list_id, std::string name) override;
    void AddFilesInLibrary(uint64_t list_id, const std::vector<std::filesystem::path> &files) override;

    size_t GetMediaListSize(uint64_t media_list_id) const override;
    const std::vector<MediaInfo> &GetMediaFiles(uint64_t list_id) const override;
    const MediaInfo &GetMediaFileInfo(uint64_t list_id, size_t file_index) const override;
    const std::vector<MediaList> &GetAllMediaLists() const override;

    std::optional<MediaIndexes> GetMediaFileIndexesByHotkey(const Hotkey &hotkey) const override;
    std::vector<std::pair<Hotkey, int>> GetGlobalHotkeys() const override;
    int GetGlobalHotkeyIdByHotkey(const Hotkey &hotkey) const override;
    const Hotkey &GetHotkeyByHotkeyId(int hotkey_id) const override;
    int GetLastHotkeyId() const override;

private:
    uint64_t next_list_id_ = 1;
    uint64_t next_file_id_ = 1;
    std::filesystem::path audio_folder_ = audio_folder_const;

    MediaJSON media_json_;
    MediaLibrary media_library_;
    MediaScanner media_scanner_;
};