#pragma once

#include <QIcon>
#include <QLabel>
#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>

class ToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBar(QWidget *parent = nullptr);
    ~ToolBar() = default;

signals:
    void MicVolumeChanged(int volume);
    void HeadphoneVolumeChanged(int volume);

private:
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