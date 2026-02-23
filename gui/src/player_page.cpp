#include "player_page.h"
#include "play_button_delegate.h"

#include <QDir>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHeaderView>

PlayerPage::PlayerPage(AudioInterfacePlayer &player, InterfaceMediaFileHandler &media_handler, QWidget *parent)
    : QWidget(parent), player_(player)
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    toolbar_widget_ = new ToolBar(this);

    connect(toolbar_widget_, &ToolBar::MicVolumeChanged,
            this, &PlayerPage::ChangeMicVolume);

    connect(toolbar_widget_, &ToolBar::HeadphoneVolumeChanged,
            this, &PlayerPage::ChangeHeadphoneVolume);

    main_layout->addWidget(toolbar_widget_);

    table_model_ = new SoundTableModel(media_handler, this);

    table_view_ = new QTableView(this);
    table_view_->setModel(table_model_);
    table_view_->setShowGrid(false);
    table_view_->setFocusPolicy(Qt::NoFocus);
    table_view_->verticalHeader()->setVisible(false);
    table_view_->setStyleSheet("QHeaderView::section {font-weight: normal;}");

    table_view_->setDragEnabled(true);
    table_view_->setAcceptDrops(true);
    table_view_->setDropIndicatorShown(true);
    table_view_->setDragDropMode(QAbstractItemView::InternalMove);

    table_view_->setSortingEnabled(true);

    connect(toolbar_widget_, &ToolBar::SortDisable, table_view_, [this](bool sort_disable)
            { table_view_->horizontalHeader()->setSectionsClickable(!sort_disable); });

    PlayButtonDelegate *play_button_delegate = new PlayButtonDelegate(this);
    table_view_->setItemDelegateForColumn(ColumnPlayButton, play_button_delegate);

    connect(play_button_delegate, &PlayButtonDelegate::PlaySound, this,
            [this](int row)
            {
                if (!table_model_->UpdateAvailability(row))
                {
                    player_.Stop();
                    return;
                }

                const MediaInfo &info = table_model_->GetFileInfo(row);
                table_model_->SetPlayingRow(row);
                player_.Play(info.path);
            });

    table_view_->horizontalHeader()->setSectionResizeMode(ColumnPlayButton, QHeaderView::Fixed);
    table_view_->setColumnWidth(ColumnPlayButton, 20);
    table_view_->setColumnWidth(ColumnName, 500);

    table_view_->horizontalHeader()->setStretchLastSection(true);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    main_layout->addWidget(table_view_);

    connect(toolbar_widget_, &ToolBar::SearchTextChanged, table_model_, &SoundTableModel::SetSearchText);
    connect(toolbar_widget_, &ToolBar::AddFileClicked, this, &PlayerPage::AddFiles);
    connect(toolbar_widget_, &ToolBar::UpArrowClicked, this, &PlayerPage::SearchUp);
    connect(toolbar_widget_, &ToolBar::DownArrowClicked, this, &PlayerPage::SearchDown);
}

void PlayerPage::ChangeMicVolume(int volume)
{
    float float_volume = volume / 100.0f;
    player_.SetVBVolume(float_volume);
}

void PlayerPage::ChangeHeadphoneVolume(int volume)
{
    float float_volume = volume / 100.0f;
    player_.SetOutVolume(float_volume);
}

void PlayerPage::AddFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Выберите аудиофайлы"),
        QDir::currentPath(),
        tr("MP3 Files (*.mp3)"));

    if (files.isEmpty())
    {
        return;
    }

    std::vector<std::filesystem::path> paths;
    paths.reserve(files.size());
    for (const QString &file : files)
    {
        paths.emplace_back(file.toStdString());
    }

    table_model_->AddFilesInLibrary(paths);
}

void PlayerPage::SearchUp()
{
    int row = table_model_->FindNextMatchRow();

    if (row < 0)
    {
        return;
    }

    QModelIndex index = table_model_->index(row, 0);
    table_view_->setCurrentIndex(index);
    table_view_->scrollTo(index, QAbstractItemView::EnsureVisible);
}

void PlayerPage::SearchDown()
{
    int row = table_model_->FindPrevMatchRow();

    if (row < 0)
    {
        return;
    }

    QModelIndex index = table_model_->index(row, 0);
    table_view_->setCurrentIndex(index);
    table_view_->scrollTo(index, QAbstractItemView::EnsureVisible);
}
