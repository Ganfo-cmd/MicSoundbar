#ifndef TEXT_TO_SPEECH_PAGE_H
#define TEXT_TO_SPEECH_PAGE_H

#include <QWidget>
// Пока класс заглушка
class TextToSpeechPage : public QWidget
{
    Q_OBJECT

public:
    explicit TextToSpeechPage(QWidget *parent = nullptr);
    ~TextToSpeechPage() = default;
};

#endif // TEXT_TO_SPEECH_PAGE_H
