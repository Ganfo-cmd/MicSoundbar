#include "mainwidget.h"
#include "player_page.h"
#include "text_to_speech_page.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>

#include <string>

MainWidget::MainWidget(AudioInterfacePlayer &player, const std::vector<MediaInfo> &media_files, QWidget *parent)
    : QWidget(parent)
{
    resize(1000, 650);
    setMinimumSize(800, 630);

    // Создание основного компоновщика
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // Добавление списка вкладок
    list_widget_ = new QListWidget(this);
    list_widget_->setFixedWidth(150);

    list_widget_->addItem(QString("MP3 файлы"));
    list_widget_->addItem(QString("Озвучка \n текста"));

    for (int i = 0; i < list_widget_->count(); ++i)
    {
        QListWidgetItem *widget_item = list_widget_->item(i);
        widget_item->setTextAlignment(Qt::AlignCenter);

        QFont text_font = widget_item->font();
        text_font.setPointSize(16);

        widget_item->setFont(text_font);
    }

    list_widget_->setGridSize(QSize(100, 90));
    list_widget_->setUniformItemSizes(true);

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
    }
    QListWidget::item:first:selected {
        background: rgba(83, 83, 83, 80); /* Такой же как :selected */
    })";

    list_widget_->setStyleSheet(list_widget_style);

    // выделение текста элементов QListWidget жирным при их выборе
    connect(list_widget_, &QListWidget::currentItemChanged, this,
            [](QListWidgetItem *current, QListWidgetItem *previous)
            {
                if (previous)
                {
                    QFont font = previous->font();
                    font.setBold(false);
                    previous->setFont(font);
                }

                if (current)
                {
                    QFont font = current->font();
                    font.setBold(true);
                    current->setFont(font);
                }
            });

    list_widget_->setCurrentRow(0);

    // Добавление содержимого вкладок
    stack_widget_ = new QStackedWidget(this);
    PlayerPage *player_page = new PlayerPage(player, media_files, this);
    TextToSpeechPage *text_to_speech_page = new TextToSpeechPage(this);

    // Тестирование
    /*for (const auto &file : vect)
    {
        player_page->AddSound(QString(file.path.c_str()));
    }*/

    stack_widget_->addWidget(player_page);
    stack_widget_->addWidget(text_to_speech_page);

    layout->addWidget(list_widget_, 1);
    layout->addWidget(stack_widget_);

    // Подключение выбора элемента в списке к страницам StackedWidget
    connect(list_widget_, &QListWidget::currentRowChanged,
            stack_widget_, &QStackedWidget::setCurrentIndex);
}
