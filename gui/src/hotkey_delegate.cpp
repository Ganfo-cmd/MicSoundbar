#include "hotkey_delegate.h"

#include <windows.h>

#include <QTimer>
#include <QLineEdit>
#include <QKeySequenceEdit>

static bool IsForbiddenKey(int key)
{
    switch (key)
    {
    case Qt::Key_Escape:
    case Qt::Key_CapsLock:
    case Qt::Key_Tab:
    case Qt::Key_Meta:
        return true;
    default:
        return false;
    }
}

static bool IsModifierKey(int key)
{
    switch (key)
    {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt:
        return true;
    default:
        return false;
    }
}

static QString HotkeyToQString(uint32_t scan_code, uint32_t modifiers)
{
    QString result;

    if (modifiers & Qt::ControlModifier)
        result += "Ctrl+";

    if (modifiers & Qt::AltModifier)
        result += "Alt+";

    if (modifiers & Qt::ShiftModifier)
        result += "Shift+";

    // 16 - 23 битовые позиции для кодов сканирования в функции GetKeyNameTextW
    LONG lParam = (scan_code << 16);
    wchar_t key_name[128] = {};
    int size = GetKeyNameTextW(lParam, key_name, sizeof(key_name) / sizeof(key_name[0]));

    if (size > 0)
    {
        result += QString::fromWCharArray(key_name);
    }

    return result;
}

HotkeyEdit::HotkeyEdit(QWidget *parent) : QLineEdit(parent)
{
    setPlaceholderText("Нажмите клавишу или комбинацию");
}

void HotkeyEdit::keyPressEvent(QKeyEvent *event)
{
    if (IsForbiddenKey(event->key()) || IsModifierKey(event->key()))
    {
        return;
    }

    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        scan_code_ = 0;
        modifiers_ = 0;
        clear();
        return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        emit editingFinished();
        return;
    }

    scan_code_ = event->nativeScanCode();
    modifiers_ = event->modifiers();

    setText(HotkeyToQString(scan_code_, modifiers_));
}

uint32_t HotkeyEdit::GetScanCode() const
{
    return scan_code_;
}

uint32_t HotkeyEdit::GetModifiers() const
{
    return modifiers_;
}

HotkeyDelegate::HotkeyDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *HotkeyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    return new HotkeyEdit(parent);
}

void HotkeyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    HotkeyEdit *edit = static_cast<HotkeyEdit *>(editor);
    QString text = index.model()->data(index, Qt::EditRole).toString();
    edit->setText(text);
}

void HotkeyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    HotkeyEdit *edit = static_cast<HotkeyEdit *>(editor);

    QVariantMap hotkey;
    hotkey["scan_code"] = edit->GetScanCode();
    hotkey["modifiers"] = edit->GetModifiers();
    hotkey["display_text"] = edit->text();

    model->setData(index, hotkey, Qt::EditRole);
}
