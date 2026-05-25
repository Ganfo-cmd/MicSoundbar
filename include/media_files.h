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

    std::optional<ChangeHotkeyResult> ChangeHotkey(size_t index, const Hotkey &hotkey) override;
    void RenameFile(size_t index, std::string new_name) override;
    void MoveFile(size_t from, size_t to) override;
    void Sort(SortField field, SortOrder order) override;
    bool UpdateAvailability(size_t row) override;
    void DeleteFile(size_t index) override;
    void SaveData() override;

    void AddFilesInLibrary(const std::vector<std::filesystem::path> &files) override;

    size_t Size() const override;
    const MediaInfo &GetMediaFileInfo(size_t index) const override;
    const std::vector<MediaInfo> &GetAllMediaInfo() const override;
    std::optional<size_t> GetMediaFileIndexByHotkey(const Hotkey &hotkey) const override;

    std::vector<std::pair<Hotkey, int>> GetGlobalHotkeys() const override;
    int GetGlobalHotkeyIdByFileId(uint64_t file_id) const override;
    size_t GetMediaFileIndexByGlobalHotkeyId(int hotkey_id) const override;
    int GetLastHotkeyId() const override;

private:
    uint64_t next_id_ = 1;
    std::filesystem::path audio_folder_ = audio_folder_const;

    MediaJSON media_json_;
    MediaLibrary media_library_;
    MediaScanner media_scanner_;
};