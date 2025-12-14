#ifndef PLAYER_PAGE_H
#define PLAYER_PAGE_H

#include "interface_player.h"

#include <QWidget>
#include <QListWidget>

class PlayerPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerPage(AudioInterfacePlayer &player, QWidget *parent = nullptr);
    ~PlayerPage() = default;

    void AddSound(const QString &name);

signals:
    void PlaySoundSignal(const QString &sound_name);

private slots:
    void PlaySound(const QString &sound_name);

private:
    QListWidget *sounds_list_ = nullptr;
    AudioInterfacePlayer &player_;
};

#endif // PLAYER_PAGE_H
