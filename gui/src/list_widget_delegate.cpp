#include "list_widget_delegate.h"

#include <QLineEdit>

ListDelegate::ListDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *ListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QLineEdit *line_edit = new QLineEdit(parent);
    line_edit->setAlignment(Qt::AlignCenter);
    return line_edit;
}

void ListDelegate::initStyleOption(QStyleOptionViewItem *option,
                                   const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    option->displayAlignment = Qt::AlignCenter;
    option->font.setPointSize(16);

    if (option->state & QStyle::State_Selected)
    {
        option->font.setBold(true);
    }
}