#pragma once

#include "interface_media_handler.h"
#include "sound_table_model.h"

#include <QWidget>
#include <QTableView>
#include <QVBoxLayout>

class SoundsTableWidget : public QWidget
{
    Q_OBJECT
public:
    SoundsTableWidget(InterfaceMediaFileHandler &media_handler, uint64_t list_id, QWidget *parent = nullptr);
    ~SoundsTableWidget() = default;

    void AddFiles(const std::vector<std::filesystem::path> &paths);
    void SetSortingEnabled(bool sort_disable);
    void FindText(const QString &text);
    void SelectPrevSearchRow();
    void SelectNextSearchRow();
    void PlayRow(int row);

signals:
    void PlayFile(const std::string &file_path);
    void StopPlaying();
    void AddGlobalHotkey(const Hotkey &hotkey, int hotkey_id);
    void RemoveGlobalHotkey(int hotkey_id);
    void DataModified();

private slots:
    void ShowContexMenu(const QPoint &pos);

private:
    QTableView *table_view_ = nullptr;
    SoundTableModel *table_model_ = nullptr;

    void InitializeUI(InterfaceMediaFileHandler &media_handler, uint64_t id);
    void InitializeTableView();
    void InitializeDelegates();
    void InitializeConnections();

    void SelectRow(int row);
};
