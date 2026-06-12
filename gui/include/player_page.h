#ifndef PLAYER_PAGE_H
#define PLAYER_PAGE_H

#include <windows.h>
#undef DeleteFile
#undef PlaySound

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
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

private slots:
    void ChangeMicVolume(int volume);
    void ChangeHeadphoneVolume(int volume);
    void AddFiles();
    void SearchUp();
    void SearchDown();

    void ShowContexMenu(const QPoint &pos);

private:
    void InitializeUI();
    void InitializeToolBar(QVBoxLayout *main_layout);
    void InitializeTable(QVBoxLayout *main_layout);
    void InitializePlayerSlider(QVBoxLayout *main_layput);
    void InitializeTableView();
    void InitializeDelegates();
    void InitializeShortcuts();
    void InitializeConnections();

    void LoadGlobalHotkeys();

    void PlayRow(int row);
    void SelectRow(int row);
    void ActivateProgressSlider();

    void RegisterServiceGlobalHotkeys();
    void UnregisterServiceGlobalHotkeys();
    void RegisterGlobalHotkey(const Hotkey &hotkey, int hotkey_id);
    void UnregisterGlobalHotkey(int hotkey_id);
    void UnregisterAllGlobalHotkeys();

    bool global_hotkey_enable_ = true;
    int global_hotkey_enable_id = 1000;
    UINT global_hotkey_enable_vk = VK_F1;

    AudioInterfacePlayer &player_;
    InterfaceMediaFileHandler &media_handler_;

    ToolBar *toolbar_widget_ = nullptr;
    QTableView *table_view_ = nullptr;
    SoundTableModel *table_model_ = nullptr;

    QSlider *progress_slider_ = nullptr;
    QTimer *progress_timer_ = nullptr;
    static constexpr int ProgressSliderScale = 1000;
};

#endif // PLAYER_PAGE_H
