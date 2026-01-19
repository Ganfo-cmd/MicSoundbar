#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "interface_player.h"
#include "media_file_info.h"

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(AudioInterfacePlayer &player, const std::vector<MediaInfo> vect, QWidget *parent = nullptr);
    ~MainWidget() = default;

private:
    QListWidget *list_widget_ = nullptr;
    QStackedWidget *stack_widget_ = nullptr;
};
#endif // MAINWIDGET_H
