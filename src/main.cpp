#include "sound_player.h"
#include "mainwidget.h"
#include "media_files.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SoundPlayer audio_player;
    MediaFileHandler handler;
    MainWidget w(audio_player, handler);

    w.show();

    return app.exec();
}