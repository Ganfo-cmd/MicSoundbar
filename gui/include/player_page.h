#ifndef PLAYER_PAGE_H
#define PLAYER_PAGE_H

#include "interface_player.h"
#include "interface_media_handler.h"
#include "sound_table_model.h"
#include "media_file_info.h"

#include <QIcon>
#include <QLabel>
#include <QWidget>
#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>

class PlayerPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerPage(AudioInterfacePlayer &player, InterfaceMediaFileHandler &media_handler, QWidget *parent = nullptr);
    ~PlayerPage() = default;

signals:
    void PlaySoundSignal(const QString &sound_path);

private slots:
    void ChangeMicVolume(int volume);
    void ChangeHeadphoneVolume(int volume);

    void MicrophoneON();
    void MicrophoneOFF();
    void HeadphoneON();
    void HeadphoneOFF();

private:
    QListWidget *sounds_list_ = nullptr;
    AudioInterfacePlayer &player_;
    InterfaceMediaFileHandler &media_handler_;
    QWidget *toolbar_widget_ = nullptr;
    SoundTableModel *table_model_ = nullptr;

    bool sync_enable_ = false;
    QCheckBox *sync_volume_checkbox_ = nullptr;

    QSlider *mic_slider_ = nullptr;
    QLabel *mic_volume_label_ = nullptr;
    QPushButton *mic_button_ = nullptr;
    QIcon icon_microphone_;
    QIcon icon_microphone_muted_;
    bool mic_muted_ = false;

    QSlider *headphones_slider_ = nullptr;
    QLabel *headphone_volume_label_ = nullptr;
    QPushButton *headphones_button_ = nullptr;
    QIcon icon_headphones_;
    QIcon icon_headphones_muted_;
    bool headphones_muted_ = false;

    void UpdateMicVolumeLabel(int value);
    void UpdateHeadphoneVolumeLabel(int value);
};

#endif // PLAYER_PAGE_H
