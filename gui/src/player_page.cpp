#include "player_page.h"
#include "play_button_delegate.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>

PlayerPage::PlayerPage(AudioInterfacePlayer &player, InterfaceMediaFileHandler &media_handler, QWidget *parent)
    : QWidget(parent), player_(player), media_handler_(media_handler)
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

    table_model_ = new SoundTableModel(this);
    table_model_->SetFiles(media_handler_.GetMediaFilesInfo());

    QTableView *table_view = new QTableView(this);
    table_view->setModel(table_model_);
    table_view->setShowGrid(false);
    table_view->setFocusPolicy(Qt::NoFocus);
    table_view->verticalHeader()->setVisible(false);
    table_view->setStyleSheet("QHeaderView::section {font-weight: normal;}");

    table_view->setDragEnabled(true);
    table_view->setAcceptDrops(true);
    table_view->setDropIndicatorShown(true);
    table_view->setDragDropMode(QAbstractItemView::InternalMove);

    table_view->setSortingEnabled(true);

    PlayButtonDelegate *play_button_delegate = new PlayButtonDelegate(this);
    table_view->setItemDelegateForColumn(ColumnPlayButton, play_button_delegate);

    connect(play_button_delegate, &PlayButtonDelegate::PlaySound, this,
            [this](int row)
            {
                const MediaInfo &info = table_model_->GetFileInfo(row);
                bool available = media_handler_.IsAvailableFile(info.path);

                table_model_->SetPlayingRow(row);
                table_model_->SetAvailableRow(row, available);

                if (!available)
                {
                    player_.Stop();
                    return;
                }

                player_.Play(info.path);
            });

    table_view->horizontalHeader()->setSectionResizeMode(ColumnPlayButton, QHeaderView::Fixed);
    table_view->setColumnWidth(ColumnPlayButton, 20);
    table_view->setColumnWidth(ColumnName, 500);

    table_view->horizontalHeader()->setStretchLastSection(true);
    table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    main_layout->addWidget(table_view);

    connect(toolbar_widget_, &ToolBar::SearchTextChanged, table_model_, &SoundTableModel::SetSearchText);
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
