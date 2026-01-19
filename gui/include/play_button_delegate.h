#pragma once

#include <QStyledItemDelegate>

class PlayButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PlayButtonDelegate(QObject *parent = nullptr);

signals:
    void PlaySound(int row) const;

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};