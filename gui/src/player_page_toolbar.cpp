#include "player_page_toolbar.h"

#include <QVBoxLayout>
#include <QStyleOptionSlider>

ToolBar::ToolBar(QWidget *parent) : QWidget(parent)
{
    Q_INIT_RESOURCE(icons);

    setFixedHeight(60);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: rgba(158, 158, 158, 1);");

    QHBoxLayout *toolbar_layout = new QHBoxLayout;
    setLayout(toolbar_layout);

    QPushButton *add_file_button = new QPushButton("Добавить файл", this);
    toolbar_layout->addWidget(add_file_button);

    connect(add_file_button, &QPushButton::clicked,
            this, &ToolBar::AddFileClicked);

    search_line_edit_ = new QLineEdit(this);
    toolbar_layout->addWidget(search_line_edit_);

    connect(search_line_edit_, &QLineEdit::textChanged, this, &ToolBar::SearchTextChanged);

    QVBoxLayout *up_and_down_button_layout = new QVBoxLayout;
    up_button_ = new QPushButton(this);
    up_button_->setFixedSize(22, 22);
    up_button_->setIcon(QIcon(":/icons/up_arrow.png"));
    up_button_->hide();

    connect(up_button_, &QPushButton::clicked,
            this, &ToolBar::UpArrowClicked);

    down_button_ = new QPushButton(this);
    down_button_->setFixedSize(22, 22);
    down_button_->setIcon(QIcon(":/icons/down_arrow.png"));
    down_button_->hide();

    connect(down_button_, &QPushButton::clicked,
            this, &ToolBar::DownArrowClicked);

    up_and_down_button_layout->addWidget(down_button_);
    up_and_down_button_layout->addWidget(up_button_);
    toolbar_layout->addLayout(up_and_down_button_layout);

    connect(search_line_edit_, &QLineEdit::textChanged, this, [this](const QString &search_text)
            { if(search_text.isEmpty())
                {
                    up_button_->hide();
                    down_button_->hide();
                } else{
                    up_button_->show();
                    down_button_->show();
                } });

    QVBoxLayout *sync_and_sort_checkbox = new QVBoxLayout;

    sync_volume_checkbox_ = new QCheckBox("Синхронизация звука", this);
    sync_volume_checkbox_->setChecked(false);
    sync_and_sort_checkbox->addWidget(sync_volume_checkbox_);

    connect(sync_volume_checkbox_, &QCheckBox::toggled, this, [this](bool enable)
            { sync_enable_ = enable; });

    sort_disable_checkbox_ = new QCheckBox("Отключить сортировку", this);
    sort_disable_checkbox_->setChecked(false);
    sync_and_sort_checkbox->addWidget(sort_disable_checkbox_);

    connect(sort_disable_checkbox_, &QCheckBox::toggled, this, &ToolBar::SortDisable);

    toolbar_layout->addLayout(sync_and_sort_checkbox);

    QVBoxLayout *volume_control_layout = new QVBoxLayout;
    QHBoxLayout *mic_volume_control_layout = new QHBoxLayout;
    QHBoxLayout *headphones_volume_control_layout = new QHBoxLayout;

    mic_button_ = new QPushButton(this);
    mic_button_->setFixedSize(22, 22);
    icon_microphone_ = QIcon(":/icons/microphone.png");
    icon_microphone_muted_ = QIcon(":/icons/microphone_muted.png");
    mic_button_->setIcon(icon_microphone_);

    mic_slider_ = new QSlider(this);
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

    connect(mic_slider_, &QSlider::valueChanged, this, [this](int volume)
            {
                if (sync_enable_)
                {
                    QSignalBlocker bloker(headphones_slider_);
                    headphones_slider_->setValue(volume);

                    if (!headphones_muted_)
                    {
                        emit HeadphoneVolumeChanged(volume);
                    }
                }

                if(!mic_muted_)
                {
                    emit MicVolumeChanged(volume);
                } });

    connect(mic_button_, &QPushButton::clicked, this, [this]()
            { mic_muted_ = !mic_muted_;
            mic_button_->setIcon(mic_muted_ ? icon_microphone_muted_ : icon_microphone_);
            int volume = mic_muted_ ? 0 : mic_slider_->value();
            emit MicVolumeChanged(volume); });

    headphones_button_ = new QPushButton(this);
    headphones_button_->setFixedSize(22, 22);
    icon_headphones_ = QIcon(":/icons/headphone.png");
    icon_headphones_muted_ = QIcon(":/icons/headphones_off.png");
    headphones_button_->setIcon(icon_headphones_);
    headphones_slider_ = new QSlider(this);
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

    connect(headphones_slider_, &QSlider::valueChanged, this, [this](int volume)
            {
                if (sync_enable_)
                {
                    QSignalBlocker bloker(mic_slider_);
                    mic_slider_->setValue(volume);

                    if (!mic_muted_)
                    {
                        emit MicVolumeChanged(volume);
                    }
                }

                if(!headphones_muted_)
                {
                    emit HeadphoneVolumeChanged(volume);
                } });

    connect(headphones_button_, &QPushButton::clicked, this, [this]()
            { headphones_muted_ = !headphones_muted_;
            headphones_button_->setIcon(headphones_muted_ ? icon_headphones_muted_ : icon_headphones_);
            int volume = headphones_muted_ ? 0 : headphones_slider_->value();
            emit HeadphoneVolumeChanged(volume); });

    volume_control_layout->addLayout(mic_volume_control_layout);
    volume_control_layout->addLayout(headphones_volume_control_layout);

    toolbar_layout->addLayout(volume_control_layout);
}

void ToolBar::UpdateMicVolumeLabel(int value)
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

void ToolBar::UpdateHeadphoneVolumeLabel(int value)
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
