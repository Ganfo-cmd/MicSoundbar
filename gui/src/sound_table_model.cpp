#include "sound_table_model.h"

#include <QString>
#include <QBrush>
#include <QFont>
#include <QKeySequence>

SoundTableModel::SoundTableModel(InterfaceMediaFileHandler &media_handler, uint64_t id, QObject *parent)
    : media_handler_(media_handler), id_(id), QAbstractTableModel(parent)
{
}

int SoundTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return media_handler_.GetMediaListSize(id_);
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

    const MediaInfo &file = media_handler_.GetMediaFileInfo(id_, index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column())
        {
        case ColumnName:
            return QString::fromStdString(file.name);
        case ColumnDuration:
        {
            int minutes = file.duration / 60;
            int seconds = file.duration % 60;
            return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
        }
        case ColumnHotKey:
        {
            return QString::fromStdString(file.hotkey.display);
        }
        }
    }

    if (role == Qt::BackgroundRole)
    {
        if (file.id == playing_file_id_)
        {
            return QBrush(QColor(180, 215, 255));
        }

        if (!search_text_.isEmpty() && search_result_set_.find(index.row()) != search_result_set_.end())
        {
            return QBrush(QColor(182, 240, 198));
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

    if (index.column() == ColumnName || index.column() == ColumnHotKey)
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
    if (!IsValidRow(source_row))
    {
        return false;
    }

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

    media_handler_.MoveFile(id_, source_row, destination_row);
    endMoveRows();

    UpdateSearchCache();
    emit DataModified();
    return true;
}

Qt::DropActions SoundTableModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool SoundTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
    {
        return false;
    }

    if (index.column() == ColumnName)
    {
        media_handler_.RenameFile(id_, index.row(), value.toString().toStdString());
        emit dataChanged(index, index, {Qt::DisplayRole});
        emit DataModified();
        return true;
    }

    if (index.column() == ColumnHotKey)
    {
        QVariantMap map = value.toMap();

        Hotkey hk;
        hk.scan_code = map["scan_code"].toUInt();
        hk.modifiers = map["modifiers"].toUInt();
        hk.display = map["display_text"].toString().toStdString();

        const MediaInfo &media_file = media_handler_.GetMediaFileInfo(id_, index.row());
        if (!media_file.hotkey.IsEmpty())
        {
            int hotkey_id = media_handler_.GetGlobalHotkeyIdByHotkey(media_file.hotkey);
            emit RemoveGlobalHotkey(hotkey_id);
        }

        auto previous_owner = media_handler_.ChangeHotkey(id_, index.row(), hk);
        if (previous_owner)
        {
            const auto &prev_owner = previous_owner.value();
            int remove_hotkey_id = prev_owner.prev_hotkey_id;
            emit RemoveGlobalHotkey(remove_hotkey_id);

            QModelIndex prev_index = this->index(static_cast<int>(prev_owner.previous_owner_index), ColumnHotKey);
            emit dataChanged(prev_index, prev_index, {Qt::DisplayRole});
        }

        if (!hk.IsEmpty())
        {
            int next_hotkey_id = media_handler_.GetLastHotkeyId();
            emit AddGlobalHotkey(hk, next_hotkey_id);
        }

        emit dataChanged(index, index, {Qt::DisplayRole});
        emit DataModified();
        return true;
    }

    return false;
}

int SoundTableModel::FindNextMatchRow()
{
    if (search_result_order_.empty())
    {
        return -1;
    }

    if (current_search_index_ == search_result_order_.size() - 1)
    {
        current_search_index_ = 0;
    }
    else
    {
        ++current_search_index_;
    }

    return search_result_order_.at(current_search_index_);
}

int SoundTableModel::FindPrevMatchRow()
{
    if (search_result_order_.empty())
    {
        return -1;
    }

    if (current_search_index_ <= 0)
    {
        current_search_index_ = search_result_order_.size() - 1;
    }
    else
    {
        --current_search_index_;
    }

    return search_result_order_.at(current_search_index_);
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
    case ColumnHotKey:
        field = SortField::Hotkey;
        break;
    default:
        return;
    }

    SortOrder sort_order =
        (order == Qt::AscendingOrder)
            ? SortOrder::Ascending
            : SortOrder::Descending;

    beginResetModel();
    media_handler_.Sort(id_, field, sort_order);
    endResetModel();

    UpdateSearchCache();
    emit DataModified();
}

const MediaInfo &SoundTableModel::GetFileInfo(int row) const
{
    return media_handler_.GetMediaFileInfo(id_, row);
}

void SoundTableModel::SetPlayingRow(int row)
{
    if (!IsValidRow(row))
    {
        return;
    }

    uint64_t new_playing_file_id = media_handler_.GetMediaFileInfo(id_, row).id;
    if (new_playing_file_id == playing_file_id_)
    {
        return;
    }

    int old_row = -1;
    if (playing_file_id_ != INVALID_ID)
    {
        for (int i = 0; i < media_handler_.GetMediaListSize(id_); ++i)
        {
            if (playing_file_id_ == media_handler_.GetMediaFileInfo(id_, i).id)
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
    return media_handler_.UpdateAvailability(id_, row);
}

void SoundTableModel::DeleteFile(int row)
{
    if (!IsValidRow(row))
    {
        return;
    }

    const MediaInfo &media_file = media_handler_.GetMediaFileInfo(id_, row);
    if (!media_file.hotkey.IsEmpty())
    {
        int hotkey_id = media_handler_.GetGlobalHotkeyIdByHotkey(media_file.hotkey);
        emit RemoveGlobalHotkey(hotkey_id);
    }

    beginRemoveRows(QModelIndex(), row, row);
    media_handler_.DeleteFile(id_, row);
    endRemoveRows();

    UpdateSearchCache();
    emit DataModified();
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

    media_handler_.AddFilesInLibrary(id_, files);

    endInsertRows();

    UpdateSearchCache();
    emit DataModified();
}

void SoundTableModel::SetSearchText(const QString &text)
{
    search_text_ = text;

    if (search_text_.isEmpty())
    {
        ClearSearchCache();
    }
    else
    {
        RebuildSearchCache();
    }

    if (rowCount() > 0)
    {
        emit dataChanged(index(0, 0), index(rowCount() - 1, ColumnCount - 1), {Qt::BackgroundRole});
    }
}

void SoundTableModel::UpdateSearchCache()
{
    if (search_text_.isEmpty() || search_result_order_.empty())
    {
        return;
    }

    RebuildSearchCache();

    if (rowCount() > 0)
    {
        emit dataChanged(index(0, 0), index(rowCount() - 1, ColumnCount - 1), {Qt::BackgroundRole});
    }
}

void SoundTableModel::RebuildSearchCache()
{
    ClearSearchCache();

    const auto &files = media_handler_.GetMediaFiles(id_);
    for (int row = 0; row < files.size(); ++row)
    {
        QString name = QString::fromUtf8(files[row].name);
        if (name.contains(search_text_, Qt::CaseInsensitive))
        {
            search_result_order_.push_back(row);
            search_result_set_.emplace(row);
        }
    }
}

void SoundTableModel::ClearSearchCache()
{
    search_result_order_.clear();
    search_result_set_.clear();
    current_search_index_ = -1;
}

bool SoundTableModel::IsValidRow(int row) const
{
    return row >= 0 && row < media_handler_.GetMediaListSize(id_);
}
