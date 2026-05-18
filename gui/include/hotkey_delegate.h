#pragma once

#include <QLineEdit>
#include <QKeyEvent>
#include <QStyledItemDelegate>

class HotkeyEdit : public QLineEdit
{
    Q_OBJECT
public:
    HotkeyEdit(QWidget *parent = nullptr);

    uint32_t GetScanCode() const;
    uint32_t GetModifiers() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    uint32_t scan_code_;
    uint32_t modifiers_;
};

class HotkeyDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    HotkeyDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};
