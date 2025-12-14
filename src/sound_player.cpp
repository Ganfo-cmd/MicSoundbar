#include "sound_player.h"

#include <chrono>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace std::string_literals;

SoundPlayer::SoundPlayer()
{
    const PaError pa_init_err = Pa_Initialize();
    if (pa_init_err != paNoError)
    {
        throw std::runtime_error("PortAudio initialization failed. Error: "s + std::string(Pa_GetErrorText(pa_init_err)));
    }
}

SoundPlayer::~SoundPlayer()
{
    Stop();
    Pa_Terminate();
}

template <typename T>
void FreeBuffer(T *&ptr)
{
    if (ptr)
    {
        delete[] ptr;
        ptr = nullptr;
    }
}

void SoundPlayer::Play(const std::string &file_path)
{
    if (stream_)
    {
        Stop();
    }

    StreamInitialization(file_path.c_str());

    const PaError pa_start_err = Pa_StartStream(stream_);
    if (pa_start_err != paNoError)
    {
        Pa_CloseStream(stream_);
        stream_ = nullptr;
        throw std::runtime_error("Failed to start audio stream. Error: "s + std::string(Pa_GetErrorText(pa_start_err)));
    }
}

void SoundPlayer::Stop()
{
    if (stream_)
    {
        if (Pa_IsStreamActive(stream_) == 1)
            Pa_StopStream(stream_);
        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }
}

void SoundPlayer::SetVolume()
{
    // заглушка
}

bool SoundPlayer::IsPlaying() const
{
    return true; // заглушка
}

void SoundPlayer::StreamInitialization(const char *file_path)
{
    long rate = 0;
    int channels = 0;
    try
    {
        decoder_ = std::make_unique<MP3Decoder>();

        decoder_->OpenFile(file_path);
        decoder_->GetFormat(rate, channels);
        decoder_->SetOutputFormat(rate, channels);
        channels_ = channels;
    }
    catch (const std::exception &)
    {
        throw;
    };

    int vb_cabel_index = GetDeviceIndex();
    PaStreamParameters output_parameters =
        {
            .device = vb_cabel_index,
            .channelCount = channels,
            .sampleFormat = paInt16,
            .suggestedLatency = Pa_GetDeviceInfo(vb_cabel_index)->defaultLowOutputLatency,
            .hostApiSpecificStreamInfo = nullptr};

    const PaError pa_format_err = Pa_IsFormatSupported(nullptr, &output_parameters, rate);
    if (pa_format_err != paNoError)
    {
        throw std::runtime_error("The format is not supported. Error: "s + std::string(Pa_GetErrorText(pa_format_err)));
    }

    const PaError pa_open_err = Pa_OpenStream(
        &stream_,
        nullptr,
        &output_parameters,
        rate,
        paFramesPerBufferUnspecified,
        paClipOff,
        &SoundPlayer::PaCallback,
        this);

    if (pa_open_err != paNoError)
    {
        throw std::runtime_error("Failed to open audio stream. Error: "s + std::string(Pa_GetErrorText(pa_open_err)));
    }
}

int SoundPlayer::PaCallback(const void *inputBuffer,
                            void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo *,
                            PaStreamCallbackFlags,
                            void *userData)
{
    auto *player = static_cast<SoundPlayer *>(userData);

    size_t bytesRead = 0;
    int err = player->decoder_->ReadFile(
        reinterpret_cast<unsigned char *>(outputBuffer),
        framesPerBuffer * player->channels_ * sizeof(short),
        bytesRead);

    if (err == MPG123_DONE)
        return paComplete;

    return paContinue;
}

int SoundPlayer::GetDeviceIndex() const
{
    int vb_cabel_index = -1;
    const int num_devices = Pa_GetDeviceCount();
    const char *target_device = "CABLE Input";

    for (int i = 0; i < num_devices; ++i)
    {
        const PaDeviceInfo *device_info = Pa_GetDeviceInfo(i);
        if (strstr(device_info->name, target_device) != nullptr)
        {
            vb_cabel_index = i;
            break;
        }
    }

    if (vb_cabel_index == -1)
    {
        throw std::runtime_error("CABLE Input device not found!");
    }

    return vb_cabel_index;
}