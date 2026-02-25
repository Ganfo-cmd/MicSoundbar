#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "interface_player.h"
#include "interface_media_handler.h"
#include "media_file_info.h"

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(AudioInterfacePlayer &player, InterfaceMediaFileHandler &media_handler, QWidget *parent = nullptr);
    ~MainWidget() = default;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QListWidget *list_widget_ = nullptr;
    QStackedWidget *stack_widget_ = nullptr;
    InterfaceMediaFileHandler &media_handler_;
};
#endif // MAINWIDGET_H
