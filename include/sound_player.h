#pragma once

#include "mp3_decoder.h"
#include "interface_player.h"

#include <portaudio.h>
#include <memory>
#include <mutex>

class SoundPlayer : public AudioInterfacePlayer
{
public:
    SoundPlayer();
    ~SoundPlayer();

    void Play(const std::string &file_path) override;
    void Stop() override;
    void SetVBVolume(float volume_level) override;
    void SetOutVolume(float volume_level) override;
    bool IsPlaying() const override; /*заглушка*/

private:
    int channels_ = 0;
    PaStream *vb_stream_ = nullptr;
    PaStream *output_stream_ = nullptr;
    std::unique_ptr<MP3Decoder> vb_decoder_;
    std::unique_ptr<MP3Decoder> output_decoder_;

    float vb_volume_ = 0.5f;
    float output_volume_ = 0.5f;
    mutable std::mutex vb_volume_mutex_;
    mutable std::mutex out_volume_mutex_;

    void StreamInitialization(const char *file_path);

    void ChangeVolume(short *buffer, size_t num_samples, float volume);

    static int VBPaCallback(const void *inputBuffer,
                            void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo *timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData);

    static int OutputPaCallback(const void *inputBuffer,
                                void *outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo *timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void *userData);

    int GetDeviceIndex() const;
};