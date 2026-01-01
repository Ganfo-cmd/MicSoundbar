#include "player_page.h"
#include "sound_file_widget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolTip>
#include <QIcon>

PlayerPage::PlayerPage(AudioInterfacePlayer &player, QWidget *parent) : QWidget(parent), /*sounds_list_(new QListWidget(this)),*/ player_(player)
{
    // Создание основного компоновщика
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    QWidget *toolbar_widget_ = new QWidget(this);
    toolbar_widget_->setFixedHeight(60);
    toolbar_widget_->setStyleSheet("background-color: rgba(158, 158, 158, 1);");

    QHBoxLayout *toolbar_layout = new QHBoxLayout(toolbar_widget_);

    QLabel *leftpath = new QLabel("Left path");
    toolbar_layout->addWidget(leftpath);

    QVBoxLayout *volume_control_layout = new QVBoxLayout;

    QHBoxLayout *mic_volume_control_layout = new QHBoxLayout;
    QHBoxLayout *headphones_volume_control_layout = new QHBoxLayout;

    mic_button_ = new QPushButton;
    mic_button_->setFixedSize(22, 22);
    icon_microphone_ = QIcon(":/icons/microphone.png");
    icon_microphone_muted_ = QIcon(":/icons/microphone_muted.png");
    mic_button_->setIcon(icon_microphone_);

    mic_slider_ = new QSlider;
    mic_slider_->setRange(0, 100);
    mic_slider_->setOrientation(Qt::Horizontal);
    mic_slider_->setFixedSize(150, 22);
    mic_slider_->setValue(10);
    mic_volume_control_layout->addWidget(mic_button_, 0, Qt::AlignVCenter);
    mic_volume_control_layout->addWidget(mic_slider_, 0, Qt::AlignVCenter);

    connect(mic_slider_, &QSlider::sliderMoved, this, [this](int value)
            {
        QPoint pos = mic_slider_->mapToGlobal(
            QPoint(mic_slider_->width() / 2 - 10, -10)
            );
        QToolTip::showText(pos, QString("%1").arg(value), mic_slider_); });

    mic_muted_ = false;
    QObject::connect(mic_button_, &QPushButton::clicked, this, [this]()
                     {
        if(mic_muted_)
        {
            mic_button_->setIcon(icon_microphone_);
            MicrophoneON();
        }
        else
        {
            mic_button_->setIcon(icon_microphone_muted_);
            MicrophoneOFF();
        }
        mic_muted_ = !mic_muted_; });

    headphones_button_ = new QPushButton;
    headphones_button_->setFixedSize(22, 22);
    icon_headphones_ = QIcon(":/icons/headphone.png");
    icon_headphones_muted_ = QIcon(":/icons/headphones_off.png");
    headphones_button_->setIcon(icon_headphones_);
    headphones_slider_ = new QSlider;
    headphones_slider_->setRange(0, 100);
    headphones_slider_->setOrientation(Qt::Horizontal);
    headphones_slider_->setFixedSize(150, 22);
    headphones_slider_->setValue(10);
    headphones_volume_control_layout->addWidget(headphones_button_, 0, Qt::AlignVCenter);
    headphones_volume_control_layout->addWidget(headphones_slider_, 0, Qt::AlignVCenter);

    connect(headphones_slider_, &QSlider::sliderMoved, this, [this](int value)
            {
        QPoint pos = headphones_slider_->mapToGlobal(
            QPoint(headphones_slider_->width() / 2 - 10, -10)
            );
        QToolTip::showText(pos, QString("%1").arg(value), headphones_slider_); });

    headphones_muted_ = false;
    QObject::connect(headphones_button_, &QPushButton::clicked, this, [this]()
                     {
        if(headphones_muted_)
        {
            headphones_button_->setIcon(icon_headphones_);
            HeadphoneON();
        }
        else
        {
            headphones_button_->setIcon(icon_headphones_muted_);
            HeadphoneOFF();
        }
        headphones_muted_ = !headphones_muted_; });

    volume_control_layout->addLayout(mic_volume_control_layout);
    volume_control_layout->addLayout(headphones_volume_control_layout);

    toolbar_layout->addLayout(volume_control_layout);

    main_layout->addWidget(toolbar_widget_);

    connect(mic_slider_, &QSlider::valueChanged, this, &PlayerPage::ChangeMicVolume);
    connect(headphones_slider_, &QSlider::valueChanged, this, &PlayerPage::ChangeHeadphoneVolume);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QString list_style = R"(
    QListWidget {
        background: transparent;
        border: none;
    }
    QListWidget::item:selected {
        background: rgb(200, 200, 200);
    }
    QListWidget::item:hover {
        background: rgb(200, 200, 200);
    })";
    sounds_list_ = new QListWidget(this);

    sounds_list_->setStyleSheet(list_style);
    layout->addWidget(sounds_list_);
    main_layout->addLayout(layout);

    connect(this, &PlayerPage::PlaySoundSignal, this, &PlayerPage::PlaySound);
}

void PlayerPage::AddSound(const QString &name)
{

    SoundFileWidget *sound_file = new SoundFileWidget(this, name);

    QListWidgetItem *item = new QListWidgetItem(sounds_list_);
    item->setSizeHint(sound_file->sizeHint());

    sounds_list_->setItemWidget(item, sound_file);

    connect(sound_file, &SoundFileWidget::PlaySound, this,
            &PlayerPage::PlaySoundSignal);
}

void PlayerPage::PlaySound(const QString &sound_name)
{
    player_.Play(sound_name.toStdString());
}

void PlayerPage::ChangeMicVolume(int volume)
{
    player_.SetVBVolume(volume * 1.0f / 100);
}

void PlayerPage::ChangeHeadphoneVolume(int volume)
{
    player_.SetOutVolume(volume * 1.0f / 100);
}

void PlayerPage::MicrophoneON()
{
    float volume = mic_slider_->value() * 1.0f / 100;
    player_.SetVBVolume(volume);
}

void PlayerPage::MicrophoneOFF()
{
    player_.SetVBVolume(0.0f);
}

void PlayerPage::HeadphoneON()
{
    float volume = headphones_slider_->value() * 1.0f / 100;
    player_.SetOutVolume(volume);
}

void PlayerPage::HeadphoneOFF()
{
    player_.SetOutVolume(0.0f);
}
