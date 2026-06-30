#pragma once

#include <QTimer>
#include <QIcon>
#include <QLabel>
#include <QWidget>
#include <QSlider>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QPushButton>

class ToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBar(QWidget *parent = nullptr);
    ~ToolBar() = default;

    void SetGlobalHotkeyEnable(bool enable);

signals:
    void MicVolumeChanged(int volume);
    void HeadphoneVolumeChanged(int volume);

    void SearchTextChanged(const QString &text);
    void SortDisable(bool disable);
    void GlobalHotkeyEnable(bool enable);

    void AddFileClicked();
    void UpArrowClicked();
    void DownArrowClicked();

private:
    struct VolumeController
    {
        QPushButton *button = nullptr;
        QSlider *slider = nullptr;
        QLabel *label = nullptr;

        QIcon icon;
        QIcon muted_icon;

        bool muted = false;
    };

    QTimer search_timer_;
    QLineEdit *search_line_edit_ = nullptr;
    QPushButton *up_button_ = nullptr;
    QPushButton *down_button_ = nullptr;

    QCheckBox *global_hotkey_checkbox_ = nullptr;

    QCheckBox *sort_disable_checkbox_ = nullptr;

    bool sync_volume_enable_ = false;
    QCheckBox *sync_volume_checkbox_ = nullptr;

    VolumeController microphone_controller_;
    VolumeController headphones_controller_;

    void InitializeUI();
    void InitializeAddFileButton(QHBoxLayout *toolbar_layout);
    void InitializeSearchBar(QHBoxLayout *toolbar_layout);
    void InitializeCheckboxes(QHBoxLayout *toolbar_layout);
    void InitializeVolumeControllers(QHBoxLayout *toolbar_layout);

    void InitializeController(VolumeController &controller, QVBoxLayout *volume_control_layout,
                              const QIcon &icon, const QIcon &muted_icon);

    void InitializeSliderLabelConnections(VolumeController &controller);
    void InitializeMicConnections();
    void InitializeHeadphonesConnections();

    void UpdateVolumeLabel(int value, VolumeController &controller);
};