#include "toolbar.h"

#include <QStyleOptionSlider>

ToolBar::ToolBar(QWidget *parent) : QWidget(parent)
{
    InitializeUI();
}

void ToolBar::InitializeUI()
{
    Q_INIT_RESOURCE(icons);

    setFixedHeight(60);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: rgba(158, 158, 158, 1);");

    QHBoxLayout *toolbar_layout = new QHBoxLayout;
    setLayout(toolbar_layout);

    InitializeAddFileButton(toolbar_layout);
    InitializeSearchBar(toolbar_layout);
    InitializeCheckboxes(toolbar_layout);
    InitializeVolumeControllers(toolbar_layout);
}

void ToolBar::InitializeAddFileButton(QHBoxLayout *toolbar_layout)
{
    QPushButton *add_file_button = new QPushButton("Добавить файл", this);
    toolbar_layout->addWidget(add_file_button);

    connect(add_file_button, &QPushButton::clicked,
            this, &ToolBar::AddFileClicked);
}

void ToolBar::InitializeSearchBar(QHBoxLayout *toolbar_layout)
{
    search_line_edit_ = new QLineEdit(this);
    toolbar_layout->addWidget(search_line_edit_);

    connect(search_line_edit_, &QLineEdit::editingFinished, this, [this]()
            { search_line_edit_->clearFocus(); });

    search_timer_.setSingleShot(true);
    search_timer_.setInterval(180);

    connect(search_line_edit_, &QLineEdit::textChanged, this, [this]()
            { search_timer_.start(); });

    connect(&search_timer_, &QTimer::timeout, this, [this]()
            { emit SearchTextChanged(search_line_edit_->text()); });

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

    QVBoxLayout *up_and_down_button_layout = new QVBoxLayout;
    up_and_down_button_layout->addWidget(up_button_);
    up_and_down_button_layout->addWidget(down_button_);

    toolbar_layout->addLayout(up_and_down_button_layout);

    connect(search_line_edit_, &QLineEdit::textChanged, this, [this](const QString &search_text)
            {   bool visible = !search_text.isEmpty();
                up_button_->setVisible(visible);
                down_button_->setVisible(visible); });
}

void ToolBar::InitializeCheckboxes(QHBoxLayout *toolbar_layout)
{
    QVBoxLayout *checkbox_layout = new QVBoxLayout;
    global_hotkey_checkbox_ = new QCheckBox("Глобальные горячие клавиши", this);
    global_hotkey_checkbox_->setChecked(true);
    checkbox_layout->addWidget(global_hotkey_checkbox_);

    connect(global_hotkey_checkbox_, &QCheckBox::toggled, this, &ToolBar::GlobalHotkeyEnable);

    sync_volume_checkbox_ = new QCheckBox("Синхронизация звука", this);
    sync_volume_checkbox_->setChecked(false);
    checkbox_layout->addWidget(sync_volume_checkbox_);

    connect(sync_volume_checkbox_, &QCheckBox::toggled, this, [this](bool enable)
            { sync_volume_enable_ = enable; });

    sort_disable_checkbox_ = new QCheckBox("Отключить сортировку", this);
    sort_disable_checkbox_->setChecked(false);
    checkbox_layout->addWidget(sort_disable_checkbox_);

    connect(sort_disable_checkbox_, &QCheckBox::toggled, this, &ToolBar::SortDisable);

    toolbar_layout->addLayout(checkbox_layout);
}

void ToolBar::InitializeVolumeControllers(QHBoxLayout *toolbar_layout)
{
    QVBoxLayout *volume_control_layout = new QVBoxLayout;

    InitializeController(microphone_controller_, volume_control_layout, QIcon(":/icons/microphone.png"), QIcon(":/icons/microphone_muted.png"));
    InitializeController(headphones_controller_, volume_control_layout, QIcon(":/icons/headphone.png"), QIcon(":/icons/headphones_off.png"));

    InitializeMicConnections();
    InitializeHeadphonesConnections();

    toolbar_layout->addLayout(volume_control_layout);
}

void ToolBar::InitializeController(VolumeController &controller, QVBoxLayout *volume_control_layout,
                                   const QIcon &icon, const QIcon &muted_icon)
{
    QHBoxLayout *slider_layout = new QHBoxLayout;

    controller.button = new QPushButton(this);
    controller.button->setFixedSize(22, 22);
    controller.icon = icon;
    controller.muted_icon = muted_icon;
    controller.button->setIcon(controller.icon);
    slider_layout->addWidget(controller.button, 0, Qt::AlignVCenter);

    controller.slider = new QSlider(this);
    controller.slider->setRange(0, 100);
    controller.slider->setOrientation(Qt::Horizontal);
    controller.slider->setFixedSize(150, 22);
    controller.slider->setValue(10);
    slider_layout->addWidget(controller.slider, 0, Qt::AlignVCenter);

    // метка громкости располагается в ручную с помощью метода UpdateVolumeLabel
    controller.label = new QLabel(this);
    controller.label->hide();
    controller.label->setFixedSize(28, 18);
    controller.label->setAlignment(Qt::AlignCenter);
    controller.label->setStyleSheet(R"(
    QLabel {
        background-color: white;
        color: rgba(138, 138, 138, 1);
        font-size: 11px;
        })");

    volume_control_layout->addLayout(slider_layout);
}

void ToolBar::InitializeMicConnections()
{
    VolumeController &controller = microphone_controller_;
    InitializeSliderLabelConnections(controller);

    connect(controller.slider, &QSlider::valueChanged, this, [this, &controller](int volume)
            {
                if (sync_volume_enable_)
                {
                    QSignalBlocker blocker(headphones_controller_.slider);
                    headphones_controller_.slider->setValue(volume);

                    if (!headphones_controller_.muted)
                    {
                        emit HeadphoneVolumeChanged(volume);
                    }
                }

                if(!controller.muted)
                {
                    emit MicVolumeChanged(volume);
                } });

    connect(controller.button, &QPushButton::clicked, this, [this, &controller]()
            { controller.muted = !controller.muted;
            controller.button->setIcon(controller.muted ? controller.muted_icon : controller.icon);
            int volume = controller.muted ? 0 : controller.slider->value();
            emit MicVolumeChanged(volume); });
}

void ToolBar::InitializeHeadphonesConnections()
{
    VolumeController &controller = headphones_controller_;
    InitializeSliderLabelConnections(controller);

    connect(controller.slider, &QSlider::valueChanged, this, [this, &controller](int volume)
            {
                if (sync_volume_enable_)
                {
                    QSignalBlocker blocker(microphone_controller_.slider);
                    microphone_controller_.slider->setValue(volume);

                    if (!microphone_controller_.muted)
                    {
                        emit MicVolumeChanged(volume);
                    }
                }

                if(!controller.muted)
                {
                    emit HeadphoneVolumeChanged(volume);
                } });

    connect(controller.button, &QPushButton::clicked, this, [this, &controller]()
            { controller.muted = !controller.muted;
            controller.button->setIcon(controller.muted ? controller.muted_icon : controller.icon);
            int volume = controller.muted ? 0 : controller.slider->value();
            emit HeadphoneVolumeChanged(volume); });
}

void ToolBar::InitializeSliderLabelConnections(VolumeController &controller)
{
    connect(controller.slider, &QSlider::sliderPressed, this, [this, &controller]()
            { UpdateVolumeLabel(controller.slider->value(), controller); });

    connect(controller.slider, &QSlider::sliderMoved, this, [this, &controller](int value)
            { UpdateVolumeLabel(value, controller); });

    connect(controller.slider, &QSlider::sliderReleased, this, [&controller]()
            { controller.label->hide(); });
}

void ToolBar::SetGlobalHotkeyEnable(bool enable)
{
    global_hotkey_checkbox_->setChecked(enable);
}

void ToolBar::UpdateVolumeLabel(int value, VolumeController &controller)
{
    auto slider = controller.slider;
    auto label = controller.label;

    QStyleOptionSlider slider_opt;
    slider_opt.initFrom(slider);
    slider_opt.minimum = slider->minimum();
    slider_opt.maximum = slider->maximum();
    slider_opt.sliderPosition = value;
    slider_opt.sliderValue = value;

    QRect handle_rect = slider->style()->subControlRect(
        QStyle::CC_Slider,
        &slider_opt,
        QStyle::SC_SliderHandle,
        slider);

    QPoint pos = slider->mapToParent(handle_rect.center());
    pos.setY(pos.y() - label->height());

    label->move(pos.x() - label->width() / 2, pos.y());
    label->setText(QString::number(value));
    label->show();
}
