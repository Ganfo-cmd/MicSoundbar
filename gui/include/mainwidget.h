#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget() = default;

private:
    QListWidget *list_widget_ = nullptr;
    QStackedWidget *stack_widget_ = nullptr;
};
#endif // MAINWIDGET_H
