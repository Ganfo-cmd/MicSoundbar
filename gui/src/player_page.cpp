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
    LoadGlobalHotkeys();
}

void PlayerPage::InitializeUI()
{
    qApp->installEventFilter(this);

    QVBoxLayout *main_layout = new QVBoxLayout(this);

    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    InitializeToolBar(main_layout);
    InitializeTable(main_layout);
    InitializePlayerSlider(main_layout);
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

void PlayerPage::InitializePlayerSlider(QVBoxLayout *main_layout)
{
    progress_timer_ = new QTimer(this);
    progress_slider_ = new QSlider(Qt::Horizontal, this);
    progress_slider_->setEnabled(false);
    progress_slider_->setStyleSheet(R"(
    QSlider::groove:horizontal {
        border: 1px solid #5c5c5c;
        height: 6px;
        margin: 2px 0;
    }

    QSlider::sub-page:horizontal {
        background: #2273cf;
    }

    QSlider::add-page:horizontal {
        background: white;
    }

    QSlider::handle:horizontal {
        background: #004797;
        width: 14px;
        margin: -4px 0;
        border-radius: 6px;
    }
    QSlider::handle:horizontal:disabled {
        background: #d6d6d6;
    }

    QSlider::sub-page:horizontal:disabled {
        background: white;
    }
    )");

    connect(progress_timer_, &QTimer::timeout, this, [this]()
            {
                if(!player_.IsPlaying())
                {
                    progress_timer_->stop();
                    progress_slider_->setValue(0);
                    progress_slider_->setEnabled(false);
                    return;                    
                }

                if (!progress_slider_->isSliderDown())
                {
                    progress_slider_->setValue(player_.GetCurrentPosition() * ProgressSliderScale);
                } });

    connect(progress_slider_, &QSlider::sliderReleased, this, [this]()
            { player_.SetPosition(static_cast<double>(progress_slider_->value()) / ProgressSliderScale); });

    main_layout->addWidget(progress_slider_);
}

void PlayerPage::InitializeTableView()
{
    table_view_ = new QTableView(this);
    table_view_->setModel(table_model_);

    table_view_->setShowGrid(false);
    table_view_->setFocusPolicy(Qt::StrongFocus);
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

    QShortcut *disable_global_hotkey = new QShortcut(QKeySequence(Qt::Key_F1), table_view_);

    connect(disable_global_hotkey, &QShortcut::activated, this, [this]()
            { toolbar_widget_->SetGlobalHotkeyEnable(!global_hotkey_enable_); });
}

void PlayerPage::InitializeConnections()
{
    connect(toolbar_widget_, &ToolBar::SortDisable, table_view_, [this](bool sort_disable)
            { table_view_->horizontalHeader()->setSectionsClickable(!sort_disable); });

    connect(toolbar_widget_, &ToolBar::SearchTextChanged, table_model_, &SoundTableModel::SetSearchText);

    connect(toolbar_widget_, &ToolBar::GlobalHotkeyEnable, this, [this](bool enable)
            { global_hotkey_enable_ = enable; 
            if(global_hotkey_enable_)
            {
                LoadGlobalHotkeys();
            }
            else
            {
                UnregisterAllGlobalHotkeys();
            } });

    connect(table_view_, &QTableView::doubleClicked,
            this, [this](const QModelIndex &index)
            {
                if (index.isValid() && index.column() == ColumnHotKey) 
                { 
                    table_view_->edit(index);
                } });

    connect(table_model_, &SoundTableModel::AddGlobalHotkey, this, [this](const Hotkey &hotkey, int hotkey_id)
            { RegisterGlobalHotkey(hotkey, hotkey_id); });

    connect(table_model_, &SoundTableModel::RemoveGlobalHotkey, this, [this](int hotkey_id)
            { UnregisterGlobalHotkey(hotkey_id); });
}

void PlayerPage::LoadGlobalHotkeys()
{
    RegisterServiceGlobalHotkeys();

    for (const auto &[hotkey, hotkey_id] : media_handler_.GetGlobalHotkeys())
    {
        RegisterGlobalHotkey(hotkey, hotkey_id);
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

    ActivateProgressSlider();
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

void PlayerPage::ActivateProgressSlider()
{
    progress_slider_->setRange(0, player_.GetDuration() * ProgressSliderScale);
    progress_slider_->setEnabled(true);
    progress_timer_->start(25);
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

bool PlayerPage::eventFilter(QObject *obj, QEvent *event)
{
    if (global_hotkey_enable_ || event->type() != QEvent::KeyPress)
        return QWidget::eventFilter(obj, event);

    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    if (key_event->isAutoRepeat())
        return QWidget::eventFilter(obj, event);

    QWidget *focus_widget = QApplication::focusWidget();
    if (focus_widget && (qobject_cast<QKeySequenceEdit *>(focus_widget) ||
                         qobject_cast<QLineEdit *>(focus_widget)))
    {
        return QWidget::eventFilter(obj, event);
    }

    Hotkey hk;
    hk.scan_code = static_cast<uint32_t>(key_event->nativeScanCode());
    hk.modifiers = static_cast<uint32_t>(key_event->modifiers());

    std::optional<size_t> index = media_handler_.GetMediaFileIndexByHotkey(hk);
    if (!index)
    {
        return QWidget::eventFilter(obj, event);
    }

    int row = static_cast<int>(index.value());
    PlayRow(row);

    return true;
}

bool PlayerPage::nativeEvent(const QByteArray &eventType,
                             void *message,
                             qintptr *result)
{
    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_HOTKEY)
    {
        switch (msg->wParam)
        {
        case 1000: // вкл/выкл глобальных клавиш
            toolbar_widget_->SetGlobalHotkeyEnable(!global_hotkey_enable_);
            return 0;
        default:
            int row = media_handler_.GetMediaFileIndexByGlobalHotkeyId(msg->wParam);
            PlayRow(row);
            return true;
        }
    }

    return QWidget::nativeEvent(eventType, message, result);
}

void PlayerPage::RegisterServiceGlobalHotkeys()
{
    /*пока что только одна служебная глобальная горячая клавиша*/
    HWND hwnd = reinterpret_cast<HWND>(winId());
    RegisterHotKey(hwnd, global_hotkey_enable_id, 0, global_hotkey_enable_vk);
}

void PlayerPage::UnregisterServiceGlobalHotkeys()
{
    UnregisterGlobalHotkey(global_hotkey_enable_id);
}

void PlayerPage::RegisterGlobalHotkey(const Hotkey &hotkey, int hotkey_id)
{
    if (!global_hotkey_enable_)
        return;

    HWND hwnd = reinterpret_cast<HWND>(winId());
    uint32_t modifiers = hotkey.modifiers;

    UINT mods = 0;
    if (modifiers & Qt::ControlModifier)
        mods |= MOD_CONTROL;

    if (modifiers & Qt::AltModifier)
        mods |= MOD_ALT;

    if (modifiers & Qt::ShiftModifier)
        mods |= MOD_SHIFT;

    UINT vk = MapVirtualKey(hotkey.scan_code, MAPVK_VSC_TO_VK);
    BOOL ok = RegisterHotKey(hwnd, hotkey_id, mods, vk);

    if (!ok)
    {
        qDebug() << "RegisterHotKey failed:" << GetLastError();
    }
}

void PlayerPage::UnregisterGlobalHotkey(int hotkey_id)
{
    HWND hwnd = reinterpret_cast<HWND>(winId());
    UnregisterHotKey(hwnd, hotkey_id);
}

void PlayerPage::UnregisterAllGlobalHotkeys()
{
    for (const auto &[hotkey, hotkey_id] : media_handler_.GetGlobalHotkeys())
    {
        UnregisterGlobalHotkey(hotkey_id);
    }
}