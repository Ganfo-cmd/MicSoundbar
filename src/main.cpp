#include "sound_player.h"
#include "mainwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SoundPlayer audio_player;
    audio_player.SetOutVolume(0.5f);
    MainWidget w(audio_player);

    w.show();

    return app.exec();
}