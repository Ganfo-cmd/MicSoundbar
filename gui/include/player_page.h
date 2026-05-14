#ifndef PLAYER_PAGE_H
#define PLAYER_PAGE_H

#include "interface_player.h"
#include "interface_media_handler.h"
#include "sound_table_model.h"
#include "media_file_info.h"
#include "player_page_toolbar.h"

#include <QTableView>
#include <QShortcut>
#include <QVBoxLayout>
#include <unordered_map>

class PlayerPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerPage(AudioInterfacePlayer &player, InterfaceMediaFileHandler &media_handler, QWidget *parent = nullptr);
    ~PlayerPage() = default;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void ChangeMicVolume(int volume);
    void ChangeHeadphoneVolume(int volume);
    void AddFiles();
    void SearchUp();
    void SearchDown();

    void ShowContexMenu(const QPoint &pos);
    void ChangeHotkey(uint64_t id, const QString &key);

private:
    struct Hotkey
    {
        int key;
        Qt::KeyboardModifiers mods;

        bool operator==(const Hotkey &other) const
        {
            return key == other.key && mods == other.mods;
        }
    };

    struct HotkeyHash
    {
        size_t operator()(const Hotkey &hk) const
        {
            size_t h1 = std::hash<int>()(hk.key);
            size_t h2 = std::hash<int>()(int(hk.mods));

            return h1 ^ (h2 << 7);
        }
    };

    void InitializeUI();
    void InitializeToolBar(QVBoxLayout *main_layout);
    void InitializeTable(QVBoxLayout *main_layout);
    void InitializeTableView();
    void InitializeDelegates();
    void InitializeShortcuts();
    void InitializeConnections();
    void LoadHotkeys();

    void PlayRow(int row);
    void SelectRow(int row);

    void RegisterHotkey(uint64_t id, const QKeySequence &seq);
    void RemoveHotkey(uint64_t id);

    AudioInterfacePlayer &player_;
    InterfaceMediaFileHandler &media_handler_;
    std::unordered_map<uint64_t, Hotkey> id_to_hotkey_;
    std::unordered_map<Hotkey, uint64_t, HotkeyHash> hotkey_to_id_;

    ToolBar *toolbar_widget_ = nullptr;
    QTableView *table_view_ = nullptr;
    SoundTableModel *table_model_ = nullptr;
};

#endif // PLAYER_PAGE_H
