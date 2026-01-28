#include "play_button_delegate.h"

#include <QApplication>
#include <QMouseEvent>

PlayButtonDelegate::PlayButtonDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void PlayButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem option_copy = option;
    initStyleOption(&option_copy, index);

    const auto &widget = option_copy.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();

    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option_copy, painter, widget);

    QStyleOptionButton button;
    button.rect = option.rect.adjusted(4, 4, -4, -4);
    button.state = QStyle::State_Enabled;
    button.icon = style->standardIcon(QStyle::SP_MediaPlay);
    button.iconSize = QSize(16, 16);

    style->drawControl(QStyle::CE_PushButton, &button, painter);
}

bool PlayButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        auto *mouse_event = static_cast<QMouseEvent *>(event);

        if (option.rect.contains(mouse_event->pos()))
        {
            emit PlaySound(index.row());
            return true;
        }
    }
    return false;
}
