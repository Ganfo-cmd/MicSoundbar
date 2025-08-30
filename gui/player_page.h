#ifndef PLAYER_PAGE_H
#define PLAYER_PAGE_H

#include <QWidget>
#include <QListWidget>

class PlayerPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerPage(QWidget *parent = nullptr);
    ~PlayerPage() = default;

    void AddSound(const QString &name);

signals:
    void PlaySoundSignal(const QString &sound_name);

private slots:
    void PlaySound(const QString &sound_name);

private:
    QListWidget *sounds_list_ = nullptr;
};

#endif // PLAYER_PAGE_H
