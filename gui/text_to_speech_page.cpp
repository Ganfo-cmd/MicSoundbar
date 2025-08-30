#include "text_to_speech_page.h"

#include <QLabel>
#include <QVBoxLayout>

TextToSpeechPage::TextToSpeechPage(QWidget *parent) : QWidget(parent)
{
    QLabel *label = new QLabel("Находится в разработке...");
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addWidget(label);
}
