#include "player_page.h"
#include "sound_file_widget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

PlayerPage::PlayerPage(AudioInterfacePlayer &player, QWidget *parent) : QWidget(parent), sounds_list_(new QListWidget(this)), player_(player)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QString list_style = R"(
    QListWidget::item:selected {
        background: rgb(200, 200, 200);
    }
    QListWidget::item:hover {
        background: rgb(200, 200, 200);
    })";

    sounds_list_->setStyleSheet(list_style);
    layout->addWidget(sounds_list_);

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
