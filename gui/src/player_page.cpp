#include "player_page.h"
#include "play_button_delegate.h"
#include "hotkey_delegate.h"

#include <QDir>
#include <QMenu>
#include <QFileDialog>
#include <QHeaderView>
#include <QProcess>
#include <QApplication>
#include <QKeyEvent>
#include <QKeySequenceEdit>

PlayerPage::PlayerPage(AudioInterfacePlayer &player, InterfaceMediaFileHandler &media_handler, QWidget *parent)
    : QWidget(parent), player_(player), media_handler_(media_handler)
{
    InitializeUI();
    LoadHotkeys();
}

void PlayerPage::InitializeUI()
{
    qApp->installEventFilter(this);

    QVBoxLayout *main_layout = new QVBoxLayout(this);

    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    InitializeToolBar(main_layout);
    InitializeTable(main_layout);
    InitializeConnections();
}

void PlayerPage::InitializeToolBar(QVBoxLayout *main_layout)
{
    toolbar_widget_ = new ToolBar(this);

    connect(toolbar_widget_, &ToolBar::MicVolumeChanged,
            this, &PlayerPage::ChangeMicVolume);

    connect(toolbar_widget_, &ToolBar::HeadphoneVolumeChanged,
            this, &PlayerPage::ChangeHeadphoneVolume);

    connect(toolbar_widget_, &ToolBar::AddFileClicked, this, &PlayerPage::AddFiles);
    connect(toolbar_widget_, &ToolBar::UpArrowClicked, this, &PlayerPage::SearchUp);
    connect(toolbar_widget_, &ToolBar::DownArrowClicked, this, &PlayerPage::SearchDown);

    main_layout->addWidget(toolbar_widget_);
}

void PlayerPage::InitializeTable(QVBoxLayout *main_layout)
{
    table_model_ = new SoundTableModel(media_handler_, this);

    InitializeTableView();
    InitializeDelegates();
    InitializeShortcuts();

    main_layout->addWidget(table_view_);
}

void PlayerPage::InitializeTableView()
{
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

    table_view_->horizontalHeader()->setSectionResizeMode(ColumnPlayButton, QHeaderView::Fixed);
    table_view_->setColumnWidth(ColumnPlayButton, 20);
    table_view_->setColumnWidth(ColumnName, 500);

    table_view_->horizontalHeader()->setStretchLastSection(true);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_view_->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(table_view_, &QTableView::customContextMenuRequested, this, &PlayerPage::ShowContexMenu);
}

void PlayerPage::InitializeDelegates()
{
    PlayButtonDelegate *play_button_delegate = new PlayButtonDelegate(this);
    table_view_->setItemDelegateForColumn(ColumnPlayButton, play_button_delegate);

    connect(play_button_delegate, &PlayButtonDelegate::PlaySound, this,
            [this](int row)
            { PlayRow(row); });

    HotkeyDelegate *hotkey_delegate = new HotkeyDelegate(this);
    table_view_->setItemDelegateForColumn(ColumnHotKey, hotkey_delegate);
}

void PlayerPage::InitializeShortcuts()
{
    QShortcut *rename_shortcut = new QShortcut(QKeySequence(Qt::Key_F2), table_view_);

    connect(rename_shortcut, &QShortcut::activated, this, [this]()
            {
                QModelIndex index = table_view_->currentIndex();
                if (index.isValid() && index.column() != ColumnHotKey)
                {
                    table_view_->edit(index);
                } });
}

void PlayerPage::InitializeConnections()
{
    connect(toolbar_widget_, &ToolBar::SortDisable, table_view_, [this](bool sort_disable)
            { table_view_->horizontalHeader()->setSectionsClickable(!sort_disable); });

    connect(toolbar_widget_, &ToolBar::SearchTextChanged, table_model_, &SoundTableModel::SetSearchText);

    connect(table_view_, &QTableView::doubleClicked,
            this, [this](const QModelIndex &index)
            {
                if (index.isValid() && index.column() == ColumnHotKey) 
                { 
                    table_view_->edit(index);
                } });

    connect(table_model_, &SoundTableModel::HotkeyChange, this, [this](uint64_t id, QKeySequence key)
            {
                if(key.isEmpty())
                {
                    RemoveHotkey(id);
                }
                else
                {
                    RegisterHotkey(id, key);
                } });
}

void PlayerPage::LoadHotkeys()
{
    for (const auto &info : media_handler_.GetAllMediaInfo())
    {
        if (!info.hotkey.empty())
        {
            ChangeHotkey(info.id, QString::fromStdString(info.hotkey));
        }
    }
}

void PlayerPage::PlayRow(int row)
{
    if (!table_model_->UpdateAvailability(row))
    {
        player_.Stop();
        return;
    }

    const MediaInfo &info = table_model_->GetFileInfo(row);
    table_model_->SetPlayingRow(row);
    player_.Play(info.path);
}

void PlayerPage::SelectRow(int row)
{
    if (row < 0)
    {
        return;
    }

    QModelIndex index = table_model_->index(row, 0);
    table_view_->setCurrentIndex(index);
    table_view_->scrollTo(index, QAbstractItemView::EnsureVisible);
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
        paths.emplace_back(QDir::toNativeSeparators(file).toStdString());
    }

    table_model_->AddFilesInLibrary(paths);
}

void PlayerPage::SearchUp()
{
    SelectRow(table_model_->FindPrevMatchRow());
}

void PlayerPage::SearchDown()
{
    SelectRow(table_model_->FindNextMatchRow());
}

void PlayerPage::ShowContexMenu(const QPoint &pos)
{
    QModelIndex index = table_view_->indexAt(pos);
    if (!index.isValid())
    {
        return;
    }

    int row = index.row();

    QMenu menu(this);
    QAction *delete_file = menu.addAction("Удалить");
    QAction *file_location = menu.addAction("Расположение файла");
    QAction *rename_file = menu.addAction("Переименовать");
    rename_file->setShortcut(QKeySequence(Qt::Key_F2));

    QAction *selected_action = menu.exec(table_view_->viewport()->mapToGlobal(pos));

    if (selected_action == delete_file)
    {
        table_model_->DeleteFile(row);
    }
    else if (selected_action == file_location)
    {
        const char *str_char = table_model_->GetFileInfo(row).path.c_str();
        QProcess::startDetached("explorer.exe", QStringList{"/select,", str_char});
    }
    else if (selected_action == rename_file)
    {
        table_view_->edit(index);
    }
}

void PlayerPage::RegisterHotkey(uint64_t id, const QKeySequence &seq)
{
    if (seq.count() != 1)
    {
        return;
    }

    QKeyCombination combo = seq[0];

    Hotkey hk;
    hk.key = combo.key();
    hk.mods = combo.keyboardModifiers();

    hotkey_to_id_[hk] = id;
    id_to_hotkey_[id] = hk;
}

void PlayerPage::RemoveHotkey(uint64_t id)
{
    auto it = id_to_hotkey_.find(id);
    if (it == id_to_hotkey_.end())
    {
        return;
    }

    hotkey_to_id_.erase(it->second);
    id_to_hotkey_.erase(it);
}

bool PlayerPage::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() != QEvent::KeyPress)
        return QWidget::eventFilter(obj, event);

    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    if (key_event->isAutoRepeat())
        return QWidget::eventFilter(obj, event);

    QWidget *focus_widget = QApplication::focusWidget();
    if (focus_widget && qobject_cast<QKeySequenceEdit *>(focus_widget))
        return QWidget::eventFilter(obj, event);

    Hotkey hk;
    hk.key = key_event->key();
    hk.mods = key_event->modifiers();

    auto it = hotkey_to_id_.find(hk);
    if (it == hotkey_to_id_.end())
    {
        return QWidget::eventFilter(obj, event);
    }

    uint64_t id = it->second;
    auto row = table_model_->GetMediaFileIndexById(id);
    PlayRow(row);

    return true;
}

void PlayerPage::ChangeHotkey(uint64_t id, const QString &key)
{
    RemoveHotkey(id);

    if (key.isEmpty())
    {
        return;
    }

    RegisterHotkey(id, QKeySequence(key));
}