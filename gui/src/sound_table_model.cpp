#include "sound_table_model.h"

#include <QString> //
#include <QBrush>

SoundTableModel::SoundTableModel(QObject *parent) : QAbstractTableModel(parent)
{
}

int SoundTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return files_.size();
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
    if (!index.isValid())
    {
        return {};
    }

    if (index.row() < 0 || index.row() >= files_.size())
    {
        return {};
    }

    const MediaInfo &file = files_[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case ColumnName:
            return QString::fromStdString(file.name);
        case ColumnDuration:
            int total_duration = static_cast<int>(file.duration);
            int minutes = total_duration / 60;
            int seconds = total_duration % 60;
            return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
        }
    }

    if (role == Qt::BackgroundRole)
    {
        if (index.row() == playing_row_)
        {
            return QBrush(QColor(180, 215, 255));
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

    if (index.isValid())
    {
        flags |= Qt::ItemIsDragEnabled;
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

    files_.move(source_row, destination_row);

    if (playing_row_ == source_row)
    {
        playing_row_ = destination_row;
    }
    else if (source_row < playing_row_ && destination_row >= playing_row_)
    {
        --playing_row_;
    }
    else if (source_row > playing_row_ && destination_row <= playing_row_)
    {
        ++playing_row_;
    }

    endMoveRows();

    return true;
}

Qt::DropActions SoundTableModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

void SoundTableModel::sort(int column, Qt::SortOrder order)
{
    if (column == ColumnPlayButton || column == ColumnHotKey)
    {
        return;
    }

    beginResetModel();

    std::sort(files_.begin(), files_.end(),
              [column, order](const MediaInfo &left, const MediaInfo &right)
              {
                bool less = false;
                switch (column)
                {
                case ColumnName:
                    less = left.name < right.name;
                    break;
                case ColumnDuration:
                    less = left.duration < right.duration;
                    break;
                default:
                    return false;
                }

                return order == Qt::AscendingOrder ? less : !less; });

    endResetModel();
}

void SoundTableModel::SetFiles(const std::vector<MediaInfo> &files)
{
    beginResetModel();

    files_.clear();
    files_.reserve(files.size());
    for (const auto &f : files)
    {
        files_.push_back(f);
    }

    endResetModel();
}

const MediaInfo &SoundTableModel::GetFileInfo(int row) const
{
    return files_[row];
}

void SoundTableModel::SetPlayingRow(int row)
{
    if (row == playing_row_)
    {
        return;
    }

    int old_row = playing_row_;
    playing_row_ = row;

    if (old_row >= 0)
    {
        emit dataChanged(index(old_row, 0), index(old_row, ColumnCount - 1));
    }

    if (playing_row_ >= 0)
    {
        emit dataChanged(index(playing_row_, 0), index(playing_row_, ColumnCount - 1));
    }
}