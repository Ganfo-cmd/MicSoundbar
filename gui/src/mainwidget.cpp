#include "mainwidget.h"
#include "list_widget_delegate.h"
#include "winapi_wrapper.h"

#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QFileDialog>
#include <QApplication>
#include <QKeyEvent>
#include <QKeySequenceEdit>
#include <QMenu>

MainWidget::MainWidget(AudioInterfacePlayer &player, InterfaceMediaFileHandler &media_handler, QWidget *parent)
    : media_handler_(media_handler), QWidget(parent)
{
    InitializeUI(player);
    LoadMediaLists();
    LoadGlobalHotkeys();
}

void MainWidget::InitializeUI(AudioInterfacePlayer &player)
{
    InitializeMainWidgetSettings();

    QHBoxLayout *main_layout = new QHBoxLayout(this);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    InitializeListWidget(main_layout);

    QVBoxLayout *table_layout = new QVBoxLayout;
    main_layout->addLayout(table_layout);

    InitializeToolBar(table_layout);
    InitializeStackedWidget(table_layout);
    InitializeControlWidget(table_layout, player);
    InitializeConnections();
}

void MainWidget::LoadMediaLists()
{
    const auto &media_lists = media_handler_.GetAllMediaLists();
    if (media_lists.empty())
    {
        uint64_t list_id = media_handler_.AddList("MP3 файлы");
        CreateList(list_id, "MP3 файлы");
    }
    else
    {
        for (const auto &media_list : media_lists)
        {
            CreateList(media_list.id, QString::fromStdString(media_list.name));
        }
    }

    list_widget_->setCurrentRow(0);
}

void MainWidget::LoadGlobalHotkeys()
{
    RegisterServiceGlobalHotkeys();

    for (const auto &[hotkey, hotkey_id] : media_handler_.GetGlobalHotkeys())
    {
        RegisterGlobalHotkey(hotkey, hotkey_id);
    }
}

void MainWidget::InitializeMainWidgetSettings()
{
    qApp->installEventFilter(this);

    resize(1000, 650);
    setMinimumSize(800, 630);

    autosave_timer_.setInterval(5000);
    autosave_timer_.setSingleShot(true);
    connect(&autosave_timer_, &QTimer::timeout, this, &MainWidget::SaveData);
}

void MainWidget::InitializeListWidget(QHBoxLayout *main_layout)
{
    list_widget_ = new QListWidget(this);
    list_widget_->setFixedWidth(150);
    list_widget_->setGridSize(QSize(100, 90));
    list_widget_->setUniformItemSizes(true);
    list_widget_->setItemDelegate(new ListDelegate(list_widget_));
    list_widget_->setContextMenuPolicy(Qt::CustomContextMenu);

    QFont font;
    font.setPointSize(16);
    list_widget_->setFont(font);

    SetupListWidgetStyle();

    QListWidgetItem *item = new QListWidgetItem("+");
    item->setData(ListRoles::RoleType, ListItemType::AddButtonItem);

    list_widget_->addItem(item);
    main_layout->addWidget(list_widget_, 1);

    connect(list_widget_, &QListWidget::itemChanged, this, &MainWidget::ListNameChanged);
    connect(list_widget_, &QListWidget::itemClicked, this, &MainWidget::ListItemAddClicked);
    connect(list_widget_, &QListWidget::customContextMenuRequested, this, &MainWidget::ShowListContextMenu);
}

void MainWidget::InitializeToolBar(QVBoxLayout *table_layout)
{
    toolbar_ = new ToolBar(this);
    table_layout->addWidget(toolbar_);

    connect(toolbar_, &ToolBar::AddFileClicked, this, &MainWidget::AddFiles);
    connect(toolbar_, &ToolBar::SortDisable, this, &MainWidget::SortDisable);
    connect(toolbar_, &ToolBar::SearchTextChanged, this, &MainWidget::SearchTextChanged);
    connect(toolbar_, &ToolBar::UpArrowClicked, this, &MainWidget::SearchUp);
    connect(toolbar_, &ToolBar::DownArrowClicked, this, &MainWidget::SearchDown);
    connect(toolbar_, &ToolBar::GlobalHotkeyEnable, this, &MainWidget::GlobalHotkeyEnable);
}

void MainWidget::InitializeControlWidget(QVBoxLayout *table_layout, AudioInterfacePlayer &player)
{
    control_widget_ = new PlayerControlWidget(player, this);
    table_layout->addWidget(control_widget_);
}

void MainWidget::InitializeStackedWidget(QVBoxLayout *table_layout)
{
    stacked_widget_ = new QStackedWidget(this);
    table_layout->addWidget(stacked_widget_);

    connect(stacked_widget_, &QStackedWidget::currentChanged, this, &MainWidget::CurrentPageChanged);
}

void MainWidget::InitializeConnections()
{
    connect(list_widget_, &QListWidget::currentRowChanged, stacked_widget_, &QStackedWidget::setCurrentIndex);

    connect(toolbar_, &ToolBar::MicVolumeChanged,
            control_widget_, &PlayerControlWidget::ChangeMicVolume);

    connect(toolbar_, &ToolBar::HeadphoneVolumeChanged,
            control_widget_, &PlayerControlWidget::ChangeHeadphoneVolume);
}

void MainWidget::AddNewList()
{
    uint64_t list_id = media_handler_.AddList("Новая вкладка");
    QListWidgetItem *item = CreateList(list_id, "Новая вкладка");

    list_widget_->setCurrentItem(item);
    list_widget_->editItem(item);

    stacked_widget_->setCurrentIndex(stacked_widget_->count() - 1);

    StartAutoSaveTimer();
}

QListWidgetItem *MainWidget::CreateList(uint64_t list_id, const QString &name)
{
    QListWidgetItem *widget_item = new QListWidgetItem(name);
    widget_item->setData(ListRoles::RoleType, ListItemType::ListItem);
    widget_item->setData(ListRoles::RoleId, list_id);
    widget_item->setFlags(widget_item->flags() | Qt::ItemIsEditable);

    list_widget_->insertItem(list_widget_->count() - 1, widget_item);

    SoundsTableWidget *sounds_table_widget = new SoundsTableWidget(media_handler_, list_id, this);
    sounds_table_widget->SetSortingEnabled(sorting_disabled_);
    stacked_widget_->addWidget(sounds_table_widget);

    connect(sounds_table_widget, &SoundsTableWidget::PlayFile, control_widget_, &PlayerControlWidget::PlayFile);
    connect(sounds_table_widget, &SoundsTableWidget::StopPlaying, control_widget_, &PlayerControlWidget::StopPlaying);
    connect(sounds_table_widget, &SoundsTableWidget::AddGlobalHotkey, this, &MainWidget::RegisterGlobalHotkey);
    connect(sounds_table_widget, &SoundsTableWidget::RemoveGlobalHotkey, this, &MainWidget::UnregisterGlobalHotkey);
    connect(sounds_table_widget, &SoundsTableWidget::DataModified, this, &MainWidget::StartAutoSaveTimer);

    return widget_item;
}

void MainWidget::ShowListContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = list_widget_->itemAt(pos);
    if (!item)
    {
        return;
    }

    int row = list_widget_->row(item);
    if (row == list_widget_->count() - 1)
    {
        return;
    }

    QMenu menu(list_widget_);
    QAction *rename_action = menu.addAction("Переименовать");
    rename_action->setShortcut(QKeySequence(Qt::Key_F2));
    QAction *delete_action = menu.addAction("Удалить");
    delete_action->setShortcut(QKeySequence(Qt::Key_Delete));

    int page_count = list_widget_->count() - 1;
    delete_action->setEnabled(page_count > 1);

    QAction *selected_action = menu.exec(list_widget_->viewport()->mapToGlobal(pos));
    if (selected_action == rename_action)
    {
        list_widget_->editItem(item);
    }
    else if (selected_action == delete_action)
    {
        std::vector<int> hotkeys_id = media_handler_.DeleteList(row);
        if (global_hotkey_enable_)
        {
            for (int hotkey_id : hotkeys_id)
            {
                UnregisterGlobalHotkey(hotkey_id);
            }
        }

        QWidget *page = stacked_widget_->widget(row);
        stacked_widget_->removeWidget(page);
        delete page;

        delete list_widget_->takeItem(row);

        if (--page_count > 0)
        {
            list_widget_->setCurrentRow(std::min(row, page_count - 1));
        }

        StartAutoSaveTimer();
    }
}

void MainWidget::SetupListWidgetStyle()
{
    QString list_widget_style = R"(
    QListWidget {
        background: qlineargradient(spread:pad, x1:0.771, y1:1, x2:0, y2:0, stop:0.145251 rgba(68, 182, 85, 255), stop:1 rgba(158, 255, 172, 255));
        border: none;
        outline: 0;
    }
    QListWidget::item {
        background: transparent;
        color: white;
        height: 90px;
        margin: 0px;
        padding: 0px;
    }
    QListWidget::item:selected {
        background: rgba(83, 83, 83, 80);
    }
    QListWidget::item:hover {
        background: rgba(83, 83, 83, 80);
    })";

    list_widget_->setStyleSheet(list_widget_style);
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    media_handler_.SaveData();
    QWidget::closeEvent(event);
}

bool MainWidget::eventFilter(QObject *obj, QEvent *event)
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

    if (!PlayFile(hk))
    {
        return QWidget::eventFilter(obj, event);
    }

    return true;
}

bool MainWidget::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_HOTKEY)
    {
        switch (msg->wParam)
        {
        case 1000: // вкл/выкл глобальных клавиш
            toolbar_->SetGlobalHotkeyEnable(!global_hotkey_enable_);
            return 0;
        default:
            PlayFile(msg->wParam);
            return true;
        }
    }

    return QWidget::nativeEvent(eventType, message, result);
}

bool MainWidget::IsAddButton(QListWidgetItem *item) const
{
    return item &&
           item->data(ListRoles::RoleType).toInt() == ListItemType::AddButtonItem;
}

void MainWidget::ListNameChanged(QListWidgetItem *item)
{
    if (IsAddButton(item))
    {
        return;
    }

    uint64_t id = item->data(ListRoles::RoleId).toULongLong();
    media_handler_.RenameList(id, item->text().toStdString());

    StartAutoSaveTimer();
}

void MainWidget::ListItemAddClicked(QListWidgetItem *item)
{
    if (IsAddButton(item))
    {
        AddNewList();
    }
}

void MainWidget::AddFiles()
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

    auto *cur_page = GetCurrentTablePage();
    if (cur_page)
    {
        cur_page->AddFiles(paths);
    }
}

void MainWidget::SortDisable(bool disable)
{
    sorting_disabled_ = disable;

    for (int i = 0; i < stacked_widget_->count(); ++i)
    {
        auto *page = qobject_cast<SoundsTableWidget *>(stacked_widget_->widget(i));
        if (page)
        {
            page->SetSortingEnabled(sorting_disabled_);
        }
    }
}

void MainWidget::SearchUp()
{
    auto *cur_page = GetCurrentTablePage();
    if (cur_page)
    {
        cur_page->SelectPrevSearchRow();
    }
}

void MainWidget::SearchDown()
{
    auto *cur_page = GetCurrentTablePage();
    if (cur_page)
    {
        cur_page->SelectNextSearchRow();
    }
}

void MainWidget::SearchTextChanged(const QString &text)
{
    search_text_ = text;

    auto *cur_page = GetCurrentTablePage();
    if (cur_page)
    {
        cur_page->FindText(text);
    }
}

void MainWidget::CurrentPageChanged(int)
{
    auto cur_page = GetCurrentTablePage();
    if (cur_page)
    {
        cur_page->FindText(search_text_);
    }
}

void MainWidget::GlobalHotkeyEnable(bool enable)
{
    global_hotkey_enable_ = enable;
    if (global_hotkey_enable_)
    {
        LoadGlobalHotkeys();
    }
    else
    {
        UnregisterAllGlobalHotkeys();
    }
}

void MainWidget::StartAutoSaveTimer()
{
    autosave_timer_.start();
}

void MainWidget::SaveData()
{
    media_handler_.SaveData();
}

SoundsTableWidget *MainWidget::GetCurrentTablePage() const
{
    return qobject_cast<SoundsTableWidget *>(stacked_widget_->currentWidget());
}

void MainWidget::RegisterServiceGlobalHotkeys()
{
    /*пока что только одна служебная глобальная горячая клавиша*/
    HWND hwnd = reinterpret_cast<HWND>(winId());
    RegisterHotKey(hwnd, global_hotkey_enable_id, 0, global_hotkey_enable_vk);
}

void MainWidget::UnregisterServiceGlobalHotkeys()
{
    UnregisterGlobalHotkey(global_hotkey_enable_id);
}

void MainWidget::RegisterGlobalHotkey(const Hotkey &hotkey, int hotkey_id)
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

void MainWidget::UnregisterGlobalHotkey(int hotkey_id)
{
    HWND hwnd = reinterpret_cast<HWND>(winId());
    UnregisterHotKey(hwnd, hotkey_id);
}

void MainWidget::UnregisterAllGlobalHotkeys()
{
    for (const auto &[hotkey, hotkey_id] : media_handler_.GetGlobalHotkeys())
    {
        UnregisterGlobalHotkey(hotkey_id);
    }
}

bool MainWidget::PlayFile(const Hotkey &hotkey)
{
    std::optional<MediaIndexes> indexes = media_handler_.GetMediaFileIndexesByHotkey(hotkey);
    if (!indexes)
    {
        return false;
    }

    const MediaIndexes &media_indexes = indexes.value();
    size_t list_index = media_indexes.list_index;
    list_widget_->setCurrentRow(list_index);

    auto *page = GetCurrentTablePage();
    if (!page)
    {
        return false;
    }

    size_t file_index = media_indexes.file_index;
    page->PlayRow(file_index);
    return true;
}

bool MainWidget::PlayFile(int hotkey_id)
{
    const Hotkey &hotkey = media_handler_.GetHotkeyByHotkeyId(hotkey_id);
    return PlayFile(hotkey);
}