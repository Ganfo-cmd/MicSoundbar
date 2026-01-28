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