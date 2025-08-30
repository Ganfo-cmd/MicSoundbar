#ifndef SOUND_FILE_WIDGET_H
#define SOUND_FILE_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QToolButton>

class SoundFileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SoundFileWidget(QWidget *parent = nullptr, QString sound_name = "файл");
    ~SoundFileWidget() = default;

    QString GetName() const;

    QSize sizeHint() const override;

signals:
    void PlaySound(const QString &sound_name);

private slots:
    void PlayBtnClicked();

private:
    QLabel *sound_label_ = nullptr;
    QToolButton *play_button_ = nullptr;

    QString sound_name_;
};

#endif // SOUND_FILE_WIDGET_H
