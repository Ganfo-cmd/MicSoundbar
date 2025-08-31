#include "sound_file_widget.h"

#include <QStyle>
#include <QToolButton>
#include <QHBoxLayout>

SoundFileWidget::SoundFileWidget(QWidget *parent, QString sound_name)
    : QWidget(parent), sound_name_(sound_name)
{
    sound_label_ = new QLabel(sound_name_);
    sound_label_->setStyleSheet("font-size: 14px");

    play_button_ = new QToolButton(this);
    play_button_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    play_button_->setToolTip("Воспроизвести");
    play_button_->setFixedSize(30, 30);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(play_button_);
    layout->addWidget(sound_label_);

    setFixedHeight(40);

    connect(play_button_, &QToolButton::clicked, this, &SoundFileWidget::PlayBtnClicked);
}

QString SoundFileWidget::GetName() const
{
    return sound_name_;
}

QSize SoundFileWidget::sizeHint() const
{
    return QSize(200, 40);
}

void SoundFileWidget::PlayBtnClicked()
{
    emit PlaySound(sound_name_);
}
