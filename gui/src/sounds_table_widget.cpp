#include "sounds_table_widget.h"
#include "play_button_delegate.h"
#include "hotkey_delegate.h"

#include <QMenu>
#include <QProcess>
#include <QHeaderView>

SoundsTableWidget::SoundsTableWidget(InterfaceMediaFileHandler &media_handler, uint64_t list_id, QWidget *parent)
    : QWidget(parent)
{
    InitializeUI(media_handler, list_id);
}

void SoundsTableWidget::InitializeUI(InterfaceMediaFileHandler &media_handler, uint64_t list_id)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    table_model_ = new SoundTableModel(media_handler, list_id, this);

    InitializeTableView();
    InitializeDelegates();
    InitializeConnections();

    layout->addWidget(table_view_);
}

void SoundsTableWidget::InitializeTableView()
{
    table_view_ = new QTableView(this);
    table_view_->setModel(table_model_);

    table_view_->setShowGrid(false);
    table_view_->setFocusPolicy(Qt::StrongFocus);
    table_view_->verticalHeader()->setVisible(false);

    table_view_->setStyleSheet("QHeaderView::section {font-weight: normal;}");

    table_view_->setDragEnabled(true);
    table_view_->setAcceptDrops(true);
    table_view_->setDropIndicatorShown(true);
    table_view_->setDragDropMode(QAbstractItemView::InternalMove);

    table_view_->setSortingEnabled(true);

    table_view_->horizontalHeader()->setSectionResizeMode(ColumnPlayButton, QHeaderView::Fixed);
    table_view_->setColumnWidth(ColumnPlayButton, 20);
    table_view_->setColumnWidth(ColumnName, 500);

    table_view_->horizontalHeader()->setStretchLastSection(true);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_view_->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(table_view_, &QTableView::customContextMenuRequested, this, &ShowContexMenu);

    connect(table_view_, &QTableView::doubleClicked,
            this, [this](const QModelIndex &index)
            {
                if (index.isValid() && index.column() == ColumnHotKey) 
                { 
                    table_view_->edit(index);
                } });
}

void SoundsTableWidget::InitializeDelegates()
{
    PlayButtonDelegate *play_button_delegate = new PlayButtonDelegate(this);
    table_view_->setItemDelegateForColumn(ColumnPlayButton, play_button_delegate);

    connect(play_button_delegate, &PlayButtonDelegate::PlaySound, this, &SoundsTableWidget::PlayRow);

    HotkeyDelegate *hotkey_delegate = new HotkeyDelegate(this);
    table_view_->setItemDelegateForColumn(ColumnHotKey, hotkey_delegate);
}

void SoundsTableWidget::InitializeConnections()
{
    connect(table_model_, &SoundTableModel::AddGlobalHotkey, this, &SoundsTableWidget::AddGlobalHotkey);
    connect(table_model_, &SoundTableModel::RemoveGlobalHotkey, this, &SoundsTableWidget::RemoveGlobalHotkey);
    connect(table_model_, &SoundTableModel::DataModified, this, SoundsTableWidget::DataModified);
}

void SoundsTableWidget::AddFiles(const std::vector<std::filesystem::path> &paths)
{
    table_model_->AddFilesInLibrary(paths);
}

void SoundsTableWidget::SetSortingEnabled(bool sort_disable)
{
    table_view_->horizontalHeader()->setSectionsClickable(!sort_disable);
}

void SoundsTableWidget::FindText(const QString &text)
{
    table_model_->SetSearchText(text);
}

void SoundsTableWidget::SelectPrevSearchRow()
{
    SelectRow(table_model_->FindPrevMatchRow());
}

void SoundsTableWidget::SelectNextSearchRow()
{
    SelectRow(table_model_->FindNextMatchRow());
}

void SoundsTableWidget::SelectRow(int row)
{
    if (row < 0)
    {
        return;
    }

    QModelIndex index = table_model_->index(row, ColumnPlayButton);
    table_view_->setCurrentIndex(index);
    table_view_->scrollTo(index, QAbstractItemView::EnsureVisible);
}

void SoundsTableWidget::ShowContexMenu(const QPoint &pos)
{
    QModelIndex index = table_view_->indexAt(pos);
    if (!index.isValid())
    {
        return;
    }

    int row = index.row();

    QMenu menu(this);
    QAction *delete_file = menu.addAction("Удалить");
    QAction *file_location = menu.addAction("Расположение файла");
    QAction *rename_file = menu.addAction("Переименовать");
    rename_file->setShortcut(QKeySequence(Qt::Key_F2));

    QAction *selected_action = menu.exec(table_view_->viewport()->mapToGlobal(pos));

    if (selected_action == delete_file)
    {
        table_model_->DeleteFile(row);
    }
    else if (selected_action == file_location)
    {
        const auto &path = table_model_->GetFileInfo(row).path;
        QProcess::startDetached("explorer.exe", QStringList{"/select,", QString::fromStdString(path)});
    }
    else if (selected_action == rename_file)
    {
        table_view_->edit(index);
    }
}

void SoundsTableWidget::PlayRow(int row)
{
    if (!table_model_->UpdateAvailability(row))
    {
        emit StopPlaying();
        return;
    }

    const MediaInfo &info = table_model_->GetFileInfo(row);

    table_model_->SetPlayingRow(row);
    emit PlayFile(info.path);
}