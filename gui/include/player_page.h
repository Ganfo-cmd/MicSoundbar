#ifndef PLAYER_PAGE_H
#define PLAYER_PAGE_H

#include "interface_player.h"

#include <QWidget>
#include <QListWidget>
#include <QPushButton>

class PlayerPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerPage(AudioInterfacePlayer &player, QWidget *parent = nullptr);
    ~PlayerPage() = default;

    void AddSound(const QString &name);

signals:
    void PlaySoundSignal(const QString &sound_name);

private slots:
    void PlaySound(const QString &sound_name);
    void ChangeMicVolume(int volume);
    void ChangeHeadphoneVolume(int volume);

    void MicrophoneON();
    void MicrophoneOFF();
    void HeadphoneON();
    void HeadphoneOFF();

private:
    QListWidget *sounds_list_ = nullptr;
    AudioInterfacePlayer &player_;
    QWidget *toolbar_widget_ = nullptr;

    QSlider *mic_slider_ = nullptr;
    QPushButton *mic_button_ = nullptr;
    QIcon icon_microphone_;
    QIcon icon_microphone_muted_;
    bool mic_muted_ = false;

    QSlider *headphones_slider_ = nullptr;
    QPushButton *headphones_button_ = nullptr;
    QIcon icon_headphones_;
    QIcon icon_headphones_muted_;
    bool headphones_muted_ = false;
};

#endif // PLAYER_PAGE_H
