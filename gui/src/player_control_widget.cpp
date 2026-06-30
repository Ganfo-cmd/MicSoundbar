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
    main_layout->setContentsMargins(0, 0, 0, 0);

    InitializeSlider(main_layout);
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
                if(!player_.IsPlaying())
                {
                    progress_timer_->stop();
                    progress_slider_->setValue(0);
                    progress_slider_->setEnabled(false);
                    return;                    
                }

                if (!progress_slider_->isSliderDown())
                {
                    progress_slider_->setValue(player_.GetCurrentPosition() * ProgressSliderScale);
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
    progress_slider_->setRange(0, player_.GetDuration() * ProgressSliderScale);
    progress_slider_->setEnabled(true);
    progress_timer_->start(25);
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