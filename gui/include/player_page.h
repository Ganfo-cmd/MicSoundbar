#ifndef PLAYER_PAGE_H
#define PLAYER_PAGE_H

#include "interface_player.h"
#include "interface_media_handler.h"
#include "sound_table_model.h"
#include "media_file_info.h"
#include "player_page_toolbar.h"

#include <QTableView>

class PlayerPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerPage(AudioInterfacePlayer &player, InterfaceMediaFileHandler &media_handler, QWidget *parent = nullptr);
    ~PlayerPage() = default;

private slots:
    void ChangeMicVolume(int volume);
    void ChangeHeadphoneVolume(int volume);
    void AddFiles();
    void SearchUp();
    void SearchDown();

private:
    ToolBar *toolbar_widget_ = nullptr;
    QTableView *table_view_ = nullptr;
    SoundTableModel *table_model_ = nullptr;

    AudioInterfacePlayer &player_;
};

#endif // PLAYER_PAGE_H
