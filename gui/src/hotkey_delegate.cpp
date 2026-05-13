#include "hotkey_delegate.h"

#include <QTimer>
#include <QLineEdit>
#include <QKeySequenceEdit>

HotkeyDelegate::HotkeyDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *HotkeyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    QKeySequenceEdit *editor = new QKeySequenceEdit(parent);
    editor->setMaximumSequenceLength(1);
    editor->setClearButtonEnabled(true);

    QTimer::singleShot(0, editor, [editor]()
    {
        if (auto *line_edit = editor->findChild<QLineEdit*>())
        {
            line_edit->setPlaceholderText(
                "Нажмите клавишу или комбинацию");
        }
    });

    return editor;
}

void HotkeyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QKeySequenceEdit *edit = static_cast<QKeySequenceEdit *>(editor);
    edit->setKeySequence(QKeySequence(text));
}

void HotkeyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QKeySequenceEdit *edit = static_cast<QKeySequenceEdit *>(editor);
    QKeySequence seq = edit->keySequence();
    QString portable_text = seq.toString(QKeySequence::PortableText);

    model->setData(index, portable_text, Qt::EditRole);
}
