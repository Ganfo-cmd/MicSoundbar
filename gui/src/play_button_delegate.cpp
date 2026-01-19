#include "play_button_delegate.h"

#include <QApplication>
#include <QMouseEvent>

PlayButtonDelegate::PlayButtonDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void PlayButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton button;
    button.rect = option.rect.adjusted(4, 4, -4, -4);
    button.state = QStyle::State_Enabled;
    button.icon = QApplication::style()->standardIcon(QStyle::SP_MediaPlay);
    button.iconSize = QSize(16, 16);

    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
}

bool PlayButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        if (option.rect.contains(mouseEvent->pos()))
        {
            emit PlaySound(index.row());
            return true;
        }
    }
    return false;
}
