#pragma once
#include "media_file_info.h"
#include "interface_media_handler.h"

#include <QAbstractTableModel>
#include <QMimeData>

static constexpr uint64_t INVALID_ID = 0;

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
    SoundTableModel(InterfaceMediaFileHandler &media_handler, QObject *parent = nullptr);
    ~SoundTableModel() = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;

    int FindNextMatchRow();
    int FindPrevMatchRow();
    void sort(int column, Qt::SortOrder order) override;

    const MediaInfo &GetFileInfo(int row) const;

    void SetPlayingRow(int row);
    bool UpdateAvailability(int row);
    void AddFilesInLibrary(const std::vector<std::filesystem::path> &files);

public slots:
    void SetSearchText(const QString &text);

private:
    QString search_text_;
    int current_search_row_ = -1;
    uint64_t playing_file_id_ = INVALID_ID;
    InterfaceMediaFileHandler &media_handler_;

    bool IsValidRow(int row) const;
};