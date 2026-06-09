#pragma once

#include <QLineEdit>
#include <QKeyEvent>
#include <QStyledItemDelegate>

static bool IsForbiddenKey(int key);

static bool IsModifierKey(int key);

static QString HotkeyToQString(uint32_t scan_code, uint32_t modifiers);

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
    uint32_t scan_code_ = 0;
    uint32_t modifiers_ = 0;
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
