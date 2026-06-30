#pragma once

#include <QStyledItemDelegate>

class ListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ListDelegate(QObject *parent = nullptr);
    ~ListDelegate() = default;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    void initStyleOption(QStyleOptionViewItem *option,
                         const QModelIndex &index) const override;

private:
};