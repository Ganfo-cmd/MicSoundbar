#include "sound_table_model.h"

#include <QString> //

SoundTableModel::SoundTableModel(QObject *parent) : QAbstractTableModel(parent)
{
}

int SoundTableModel::rowCount(const QModelIndex &parent) const
{
    /*if (parent.isValid())
        return 0;*/

    return files_.size();
}

int SoundTableModel::columnCount(const QModelIndex &parent) const
{
    /*if (parent.isValid())
        return 0;*/

    return ColumnCount;
}

QVariant SoundTableModel::data(const QModelIndex &index, int role) const
{
    /*if (!index.isValid())
        return {};

    if (index.row() < 0 || index.row() >= files_.size())
        return {};*/

    const MediaInfo &file = files_[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case ColumnName:
            return QString::fromStdString(file.name);
        case ColumnDuration:
            return file.duration;
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