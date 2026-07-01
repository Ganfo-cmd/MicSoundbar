#include "player_control_widget.h"

PlayerControlWidget::PlayerControlWidget(AudioInterfacePlayer &player, QWidget *parent)
    : player_(player), QWidget(parent)
{
    InitializeUI();
}

void PlayerControlWidget::InitializeUI()
{
    QHBoxLayout *main_layout = new QHBoxLayout(this);

    main_layout->setSpacing(0);
    main_layout->setContentsMargins(5, 0, 0, 0);

    InitializeDuration(main_layout);
    InitializeSlider(main_layout);
}

void PlayerControlWidget::InitializeDuration(QHBoxLayout *main_layout)
{
    duration_label_ = new QLabel("00:00 / 00:00", this);
    duration_label_->setFixedWidth(70);

    main_layout->addWidget(duration_label_);
}

void PlayerControlWidget::InitializeSlider(QHBoxLayout *main_layout)
{
    progress_timer_ = new QTimer(this);
    progress_slider_ = new QSlider(Qt::Horizontal, this);
    progress_slider_->setEnabled(false);
    progress_slider_->setStyleSheet(R"(
    QSlider::groove:horizontal {
        border: 1px solid #5c5c5c;
        height: 6px;
        margin: 2px 0;
    }

    QSlider::sub-page:horizontal {
        background: #2273cf;
    }

    QSlider::add-page:horizontal {
        background: white;
    }

    QSlider::handle:horizontal {
        background: #004797;
        width: 14px;
        margin: -4px 0;
        border-radius: 6px;
    }
    QSlider::handle:horizontal:disabled {
        background: #d6d6d6;
    }

    QSlider::sub-page:horizontal:disabled {
        background: white;
    }
    )");

    connect(progress_timer_, &QTimer::timeout, this, [this]()
            {
                if (!player_.IsPlaying())
                {
                    progress_timer_->stop();
                    progress_slider_->setValue(0);
                    progress_slider_->setEnabled(false);

                    last_second_ = -1;
                    duration_label_->setText("00:00 / 00:00");

                    return;
                }

                int current;
                if (progress_slider_->isSliderDown())
                {
                    current = progress_slider_->value() / ProgressSliderScale;
                }
                else
                {
                    double position = player_.GetCurrentPosition();
                    progress_slider_->setValue(position * ProgressSliderScale);

                    current = static_cast<int>(position);
                }

               
                if (current != last_second_)
                {
                    last_second_ = current;
                    UpdateDurationLabel(current, static_cast<int>(player_.GetDuration()));
                } });

    connect(progress_slider_, &QSlider::sliderReleased, this, [this]()
            { player_.SetPosition(static_cast<double>(progress_slider_->value()) / ProgressSliderScale); });

    main_layout->addWidget(progress_slider_);
}

void PlayerControlWidget::PlayFile(const std::string &file_path)
{
    player_.Play(file_path);
    ActivateProgressSlider();
}

void PlayerControlWidget::StopPlaying()
{
    player_.Stop();
}

void PlayerControlWidget::ActivateProgressSlider()
{
    double total_duration = player_.GetDuration();
    progress_slider_->setRange(0, total_duration * ProgressSliderScale);
    progress_slider_->setEnabled(true);

    UpdateDurationLabel(0, static_cast<int>(total_duration));

    progress_timer_->start(25);
}

void PlayerControlWidget::UpdateDurationLabel(int current, int total)
{
    auto format = [](int sec)
    {
        return QString("%1:%2")
            .arg(sec / 60)
            .arg(sec % 60, 2, 10, QChar('0'));
    };

    duration_label_->setText(format(current) + " / " + format(total));
}

void PlayerControlWidget::ChangeMicVolume(int volume)
{
    float float_volume = volume / 100.0f;
    player_.SetVBVolume(float_volume);
}

void PlayerControlWidget::ChangeHeadphoneVolume(int volume)
{
    float float_volume = volume / 100.0f;
    player_.SetOutVolume(float_volume);
}