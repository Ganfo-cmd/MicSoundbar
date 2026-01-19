#include "player_page.h"
#include "sound_table_model.h"
#include "play_button_delegate.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>

PlayerPage::PlayerPage(AudioInterfacePlayer &player, const std::vector<MediaInfo> &media_files, QWidget *parent) : QWidget(parent), /*sounds_list_(new QListWidget(this)),*/ player_(player)
{
    Q_INIT_RESOURCE(icons);
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

    sync_volume_checkbox_ = new QCheckBox("Синхронизация звука");
    sync_volume_checkbox_->setChecked(false);
    toolbar_layout->addWidget(sync_volume_checkbox_);

    connect(sync_volume_checkbox_, &QCheckBox::toggled, this, [this](bool enable)
            { sync_enable_ = enable; });

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

    mic_volume_label_ = new QLabel(this);
    mic_volume_label_->hide();
    mic_volume_label_->setFixedSize(28, 18);
    mic_volume_label_->setAlignment(Qt::AlignCenter);
    mic_volume_label_->setStyleSheet(R"(
    QLabel {
        background-color: white;
        color: rgba(138, 138, 138, 1);
        font-size: 11px;
        })");

    connect(mic_slider_, &QSlider::sliderPressed, this, [this]()
            { UpdateMicVolumeLabel(mic_slider_->value()); });

    connect(mic_slider_, &QSlider::sliderMoved, this, [this](int value)
            { UpdateMicVolumeLabel(value); });

    connect(mic_slider_, &QSlider::sliderReleased, this, [this]()
            { mic_volume_label_->hide(); });

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

    headphone_volume_label_ = new QLabel(this);
    headphone_volume_label_->hide();
    headphone_volume_label_->setFixedSize(28, 18);
    headphone_volume_label_->setAlignment(Qt::AlignCenter);
    headphone_volume_label_->setStyleSheet(R"(
    QLabel {
        background-color: white;
        color: rgba(138, 138, 138, 1);
        font-size: 11px;
        })");

    connect(headphones_slider_, &QSlider::sliderPressed, this, [this]()
            { UpdateHeadphoneVolumeLabel(headphones_slider_->value()); });

    connect(headphones_slider_, &QSlider::sliderMoved, this, [this](int value)
            { UpdateHeadphoneVolumeLabel(value); });

    connect(headphones_slider_, &QSlider::sliderReleased, this, [this]()
            { headphone_volume_label_->hide(); });

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

    SoundTableModel *table_model = new SoundTableModel(this);
    table_model->SetFiles(media_files);

    QTableView *table_view = new QTableView(this);
    table_view->setModel(table_model);

    PlayButtonDelegate *play_button_delegate = new PlayButtonDelegate(this);
    table_view->setItemDelegateForColumn(ColumnPlayButton, play_button_delegate);

    connect(play_button_delegate, &PlayButtonDelegate::PlaySound, this,
            [this, table_model](int row)
            {
                const MediaInfo &info = table_model->GetFileInfo(row);
                player_.Play(info.path);
            });

    table_view->horizontalHeader()->setSectionResizeMode(ColumnPlayButton, QHeaderView::Fixed);
    table_view->setColumnWidth(ColumnPlayButton, 20);
    table_view->setColumnWidth(ColumnName, 500);

    table_view->horizontalHeader()->setStretchLastSection(true);
    table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(table_view);
    main_layout->addLayout(layout);

    connect(this, &PlayerPage::PlaySoundSignal, this, &PlayerPage::PlaySound);
}

void PlayerPage::PlaySound(const QString &sound_name)
{
    player_.Play(sound_name.toStdString());
}

void PlayerPage::ChangeMicVolume(int volume)
{
    float float_volume = volume * 1.0f / 100;
    if (sync_enable_)
    {
        QSignalBlocker bloker(headphones_slider_);
        headphones_slider_->setValue(volume);

        if (!headphones_muted_)
        {
            player_.SetOutVolume(float_volume);
        }
    }

    if (!mic_muted_)
    {
        player_.SetVBVolume(float_volume);
    }
}

void PlayerPage::ChangeHeadphoneVolume(int volume)
{
    float float_volume = volume * 1.0f / 100;
    if (sync_enable_)
    {
        QSignalBlocker bloker(mic_slider_);
        mic_slider_->setValue(volume);

        if (!mic_muted_)
        {
            player_.SetVBVolume(float_volume);
        }
    }

    if (!headphones_muted_)
    {
        player_.SetOutVolume(float_volume);
    }
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

void PlayerPage::UpdateMicVolumeLabel(int value)
{
    QStyleOptionSlider slider_opt;
    slider_opt.initFrom(mic_slider_);
    slider_opt.minimum = mic_slider_->minimum();
    slider_opt.maximum = mic_slider_->maximum();
    slider_opt.sliderPosition = value;
    slider_opt.sliderValue = value;

    QRect handle_rect = mic_slider_->style()->subControlRect(
        QStyle::CC_Slider,
        &slider_opt,
        QStyle::SC_SliderHandle,
        mic_slider_);

    QPoint pos = mic_slider_->mapToParent(handle_rect.center());
    pos.setY(pos.y() - 20);

    mic_volume_label_->move(pos.x() - mic_volume_label_->width() / 2, pos.y());
    mic_volume_label_->setText(QString::number(value));
    mic_volume_label_->show();
}

void PlayerPage::UpdateHeadphoneVolumeLabel(int value)
{
    QStyleOptionSlider slider_opt;
    slider_opt.initFrom(headphones_slider_);
    slider_opt.minimum = headphones_slider_->minimum();
    slider_opt.maximum = headphones_slider_->maximum();
    slider_opt.sliderPosition = value;
    slider_opt.sliderValue = value;

    QRect handle_rect = headphones_slider_->style()->subControlRect(
        QStyle::CC_Slider,
        &slider_opt,
        QStyle::SC_SliderHandle,
        headphones_slider_);

    QPoint pos = headphones_slider_->mapToParent(handle_rect.center());
    pos.setY(pos.y() - headphones_slider_->height());

    headphone_volume_label_->move(pos.x() - headphone_volume_label_->width() / 2, pos.y());
    headphone_volume_label_->setText(QString::number(value));
    headphone_volume_label_->show();
}
