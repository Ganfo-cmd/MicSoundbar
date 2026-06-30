#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <windows.h>
#undef DeleteFile
#undef PlaySound
#undef FindText

#include "interface_player.h"
#include "interface_media_handler.h"
#include "media_file_info.h"
#include "toolbar.h"
#include "player_control_widget.h"
#include "sounds_table_widget.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>

enum ListItemType
{
    AddButtonItem,
    ListItem
};

enum ListRoles
{
    RoleType = Qt::UserRole,
    RoleId
};

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(AudioInterfacePlayer &player, InterfaceMediaFileHandler &media_handler, QWidget *parent = nullptr);
    ~MainWidget() = default;

protected:
    void closeEvent(QCloseEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void ListNameChanged(QListWidgetItem *item);
    void ListItemAddClicked(QListWidgetItem *item);

    void AddFiles();
    void SortDisable(bool disable);

    void SearchUp();
    void SearchDown();
    void SearchTextChanged(const QString &text);
    void CurrentPageChanged(int);

    void GlobalHotkeyEnable(bool enable);

    void ShowListContextMenu(const QPoint &pos);

private:
    InterfaceMediaFileHandler &media_handler_;

    QString search_text_;
    bool sorting_disabled_ = false;
    QListWidget *list_widget_ = nullptr;
    QStackedWidget *stacked_widget_ = nullptr;

    bool global_hotkey_enable_ = true;
    int global_hotkey_enable_id = 1000;
    UINT global_hotkey_enable_vk = VK_F1;

    ToolBar *toolbar_ = nullptr;
    PlayerControlWidget *control_widget_ = nullptr;

    void InitializeUI(AudioInterfacePlayer &player);
    void InitializeListWidget(QHBoxLayout *main_layout);
    void InitializeToolBar(QVBoxLayout *table_layout);
    void InitializeControlWidget(QVBoxLayout *table_layout, AudioInterfacePlayer &player);
    void InitializeStackedWidget(QVBoxLayout *table_layout);
    void InitializeConnections();

    void SetupListWidgetStyle();

    void LoadMediaLists();
    void LoadGlobalHotkeys();

    void AddNewList();
    QListWidgetItem *CreateList(uint64_t list_id, const QString &name);    

    bool IsAddButton(QListWidgetItem *item) const;

    void RegisterServiceGlobalHotkeys();
    void UnregisterServiceGlobalHotkeys();
    void RegisterGlobalHotkey(const Hotkey &hotkey, int hotkey_id);
    void UnregisterGlobalHotkey(int hotkey_id);
    void UnregisterAllGlobalHotkeys();

    bool PlayFile(int hotkey_id);
    bool PlayFile(const Hotkey &hotkey);

    SoundsTableWidget *GetCurrentTablePage() const;
};
#endif // MAINWIDGET_H
