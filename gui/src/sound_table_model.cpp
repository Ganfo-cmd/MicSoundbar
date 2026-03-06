#include "sound_table_model.h"

#include <QString>
#include <QBrush>
#include <QFont>

SoundTableModel::SoundTableModel(InterfaceMediaFileHandler &media_handler, QObject *parent)
    : media_handler_(media_handler), QAbstractTableModel(parent)
{
    autosave_timer_.setInterval(5000);
    autosave_timer_.setSingleShot(true);
    connect(&autosave_timer_, &QTimer::timeout, this, &SoundTableModel::SaveData);
}

int SoundTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return media_handler_.Size();
}

int SoundTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return ColumnCount;
}

QVariant SoundTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !IsValidRow(index.row()))
    {
        return {};
    }

    const MediaInfo &file = media_handler_.GetMediaFileInfo(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column())
        {
        case ColumnName:
            return QString::fromStdString(file.name);
        case ColumnDuration:
            int minutes = file.duration / 60;
            int seconds = file.duration % 60;
            return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
        }
    }

    if (role == Qt::BackgroundRole)
    {
        if (file.id == playing_file_id_)
        {
            return QBrush(QColor(180, 215, 255));
        }

        if (!search_text_.isEmpty())
        {
            QString name = QString::fromUtf8(file.name.c_str());
            if (name.contains(search_text_, Qt::CaseInsensitive))
            {
                return QBrush(QColor(182, 240, 198));
            }
        }
    }

    if (!file.available)
    {
        if (role == Qt::ForegroundRole)
        {
            return QBrush(Qt::gray);
        }

        if (role == Qt::FontRole)
        {
            QFont font;
            font.setStrikeOut(true);
            return font;
        }
    }

    return {};
}

QVariant SoundTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case ColumnPlayButton:
            return QStringLiteral();
        case ColumnName:
            return QStringLiteral("Название");
        case ColumnDuration:
            return QStringLiteral("Длительность");
        case ColumnHotKey:
            return QStringLiteral("Горячая клавиша");
        }
    }

    return {};
}

Qt::ItemFlags SoundTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    flags |= Qt::ItemIsDropEnabled;

    if (!index.isValid())
    {
        return flags;
    }

    flags |= Qt::ItemIsDragEnabled;

    if (index.column() == ColumnName)
    {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

QStringList SoundTableModel::mimeTypes() const
{
    return {"application/x-sound-row"};
}

QMimeData *SoundTableModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.empty())
    {
        return nullptr;
    }

    int row = indexes.first().row();
    if (!IsValidRow(row))
    {
        return nullptr;
    }

    QMimeData *mime = new QMimeData;
    mime->setData("application/x-sound-row",
                  QByteArray::number(row));
    return mime;
}

bool SoundTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                   int row, int column, const QModelIndex &parent)
{
    if (action != Qt::MoveAction || !data->hasFormat("application/x-sound-row"))
    {
        return false;
    }

    int source_row = data->data("application/x-sound-row").toInt();
    int destination_row = row;
    if (destination_row == -1 && parent.isValid())
    {
        destination_row = parent.row();
    }

    bool invalid_row = destination_row < 0 || destination_row > rowCount();
    if (destination_row == source_row || invalid_row)
    {
        return false;
    }

    beginMoveRows(QModelIndex(), source_row, source_row,
                  QModelIndex(), destination_row > source_row ? destination_row + 1 : destination_row);

    media_handler_.MoveFile(source_row, destination_row);
    endMoveRows();

    StartAutosaveTimer();
    return true;
}

Qt::DropActions SoundTableModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool SoundTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.column() == ColumnName)
    {
        media_handler_.RenameFile(index.row(), value.toString().toStdString());
        emit dataChanged(index, index, {Qt::DisplayRole});
        return true;
    }

    return false;
}

int SoundTableModel::FindNextMatchRow()
{
    if (search_text_.isEmpty())
    {
        return -1;
    }

    const auto &files = media_handler_.GetAllMediaInfo();
    for (int row = current_search_row_ + 1; row < media_handler_.Size(); ++row)
    {
        const auto &file = files[row];
        QString name = QString::fromUtf8(file.name.c_str());
        if (name.contains(search_text_, Qt::CaseInsensitive))
        {
            current_search_row_ = row;
            return row;
        }
    }

    return -1;
}

int SoundTableModel::FindPrevMatchRow()
{
    if (search_text_.isEmpty())
    {
        return -1;
    }

    const auto &files = media_handler_.GetAllMediaInfo();
    for (int row = current_search_row_ - 1; row >= 0; --row)
    {
        const auto &file = files[row];
        QString name = QString::fromUtf8(file.name.c_str());
        if (name.contains(search_text_, Qt::CaseInsensitive))
        {
            current_search_row_ = row;
            return row;
        }
    }

    return -1;
}

void SoundTableModel::sort(int column, Qt::SortOrder order)
{
    SortField field;
    switch (column)
    {
    case ColumnName:
        field = SortField::Name;
        break;
    case ColumnDuration:
        field = SortField::Duration;
        break;
    default:
        return;
    }

    SortOrder sort_order =
        (order == Qt::AscendingOrder)
            ? SortOrder::Ascending
            : SortOrder::Descending;

    beginResetModel();
    media_handler_.Sort(field, sort_order);
    endResetModel();

    StartAutosaveTimer();
}

const MediaInfo &SoundTableModel::GetFileInfo(int row) const
{
    return media_handler_.GetMediaFileInfo(row);
}

void SoundTableModel::SetPlayingRow(int row)
{
    if (!IsValidRow(row))
    {
        return;
    }

    uint64_t new_playing_file_id = media_handler_.GetMediaFileInfo(row).id;
    if (new_playing_file_id == playing_file_id_)
    {
        return;
    }

    int old_row = -1;
    if (playing_file_id_ != INVALID_ID)
    {
        for (int i = 0; i < media_handler_.Size(); ++i)
        {
            if (playing_file_id_ == media_handler_.GetMediaFileInfo(i).id)
            {
                old_row = i;
                break;
            }
        }
    }

    playing_file_id_ = new_playing_file_id;
    emit dataChanged(index(row, 0), index(row, ColumnCount - 1), {Qt::BackgroundRole});

    if (old_row != -1)
    {
        emit dataChanged(index(old_row, 0), index(old_row, ColumnCount - 1), {Qt::BackgroundRole});
    }
}

bool SoundTableModel::UpdateAvailability(int row)
{
    if (!IsValidRow(row))
    {
        return false;
    }

    emit dataChanged(index(row, 0), index(row, ColumnCount - 1), {Qt::ForegroundRole, Qt::FontRole});
    return media_handler_.UpdateAvailability(row);
}

void SoundTableModel::DeleteFile(int row)
{
    if (!IsValidRow(row))
    {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    media_handler_.DeleteFile(row);
    endRemoveRows();

    autosave_timer_.start();
}

void SoundTableModel::AddFilesInLibrary(const std::vector<std::filesystem::path> &files)
{
    int old_size = rowCount();
    int files_count = files.size();

    if (files_count == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), old_size, old_size + files_count - 1);

    media_handler_.AddFilesInLibrary(files);

    endInsertRows();

    StartAutosaveTimer();
}

void SoundTableModel::SaveData()
{
    media_handler_.SaveData();
}

void SoundTableModel::SetSearchText(const QString &text)
{
    search_text_ = text;
    if (media_handler_.Size() == 0)
    {
        return;
    }

    emit dataChanged(index(0, 0), index(rowCount() - 1, ColumnCount - 1), {Qt::BackgroundRole});
}

bool SoundTableModel::IsValidRow(int row) const
{
    if (row >= 0 || row < media_handler_.Size())
    {
        return true;
    }

    return false;
}

void SoundTableModel::StartAutosaveTimer()
{
    autosave_timer_.start();
}