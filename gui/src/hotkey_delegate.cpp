#include "hotkey_delegate.h"

#include <QTimer>
#include <QLineEdit>
#include <QKeySequenceEdit>

HotkeyEdit::HotkeyEdit(QWidget *parent) : QLineEdit(parent)
{
    setPlaceholderText("Нажмите клавишу или комбинацию");
}

void HotkeyEdit::keyPressEvent(QKeyEvent *event)
{
    scan_code_ = event->nativeScanCode();
    modifiers_ = event->modifiers();

    QKeySequence seq(modifiers_ | event->key());
    setText(seq.toString(QKeySequence::PortableText));
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
