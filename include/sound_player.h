#pragma once

#include "mp3_decoder.h"
#include "interface_player.h"

#include <portaudio.h>
#include <memory>

class SoundPlayer : public AudioInterfacePlayer
{
public:
    SoundPlayer();
    ~SoundPlayer();

    void Play(const std::string &file_path) override;
    void Stop() override;
    void SetVolume() override;       /*заглушка*/
    bool IsPlaying() const override; /*заглушка*/

private:
    int channels_ = 0;
    PaStream *stream_ = nullptr;
    std::unique_ptr<MP3Decoder> decoder_;

    void StreamInitialization(const char *file_path);

    static int PaCallback(const void *inputBuffer,
                          void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData);

    int GetDeviceIndex() const;
};