#pragma once

#include "interface_player.h"

#include <QWidget>
#include <QSlider>
#include <QTimer>
#include <QHBoxLayout>
#include <QLabel>

class PlayerControlWidget : public QWidget
{
    Q_OBJECT
public:
    PlayerControlWidget(AudioInterfacePlayer &player, QWidget *parent = nullptr);
    ~PlayerControlWidget() = default;

public slots:
    void PlayFile(const std::string &file_path);
    void StopPlaying();

    void ChangeMicVolume(int volume);
    void ChangeHeadphoneVolume(int volume);

private:
    AudioInterfacePlayer &player_;

    int last_second_ = -1;
    QLabel *duration_label_ = nullptr;

    QSlider *progress_slider_ = nullptr;
    QTimer *progress_timer_ = nullptr;
    static constexpr int ProgressSliderScale = 1000;

    void InitializeUI();
    void InitializeDuration(QHBoxLayout *main_layout);
    void InitializeSlider(QHBoxLayout *main_layout);

    void ActivateProgressSlider();
    void UpdateDurationLabel(int current, int total);
};
