#pragma once
#include "media_file_info.h"

#include <QAbstractTableModel>
#include <QVector>

enum Columns
{
    ColumnPlayButton,
    ColumnName,
    ColumnDuration,
    ColumnHotKey,
    ColumnCount
};

class SoundTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SoundTableModel(QObject *parent = nullptr);
    ~SoundTableModel() = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void SetFiles(const std::vector<MediaInfo> &files);
    const MediaInfo &GetFileInfo(int row) const;

    void SetPlayingRow(int row);

private:
    int playing_row_ = -1;
    QVector<MediaInfo> files_;
};