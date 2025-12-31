#include "sound_player.h"

#include <cstring>
#include <stdexcept>

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
    if (vb_stream_ || output_stream_)
    {
        Stop();
    }

    StreamInitialization(file_path.c_str());

    const PaError vb_pa_start_err = Pa_StartStream(vb_stream_);
    const PaError out_pa_start_err = Pa_StartStream(output_stream_);
    if (vb_pa_start_err != paNoError || out_pa_start_err != paNoError)
    {
        Pa_CloseStream(vb_stream_);
        vb_stream_ = nullptr;

        Pa_CloseStream(output_stream_);
        output_stream_ = nullptr;
        throw std::runtime_error("Failed to start audio stream."s);
    }
}

void SoundPlayer::Stop()
{
    if (vb_stream_)
    {
        if (Pa_IsStreamActive(vb_stream_) == 1)
        {
            Pa_StopStream(vb_stream_);
        }
        Pa_CloseStream(vb_stream_);
        vb_stream_ = nullptr;
    }

    if (output_stream_)
    {
        if (Pa_IsStreamActive(output_stream_) == 1)
        {
            Pa_StopStream(output_stream_);
        }
        Pa_CloseStream(output_stream_);
        output_stream_ = nullptr;
    }
}

void SoundPlayer::SetVBVolume(float volume_level)
{
    std::lock_guard<std::mutex> lock(vb_volume_mutex_);
    vb_volume_ = volume_level;
}

void SoundPlayer::SetOutVolume(float volume_level)
{
    std::lock_guard<std::mutex> lock(out_volume_mutex_);
    output_volume_ = volume_level;
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
        vb_decoder_ = std::make_unique<MP3Decoder>();

        vb_decoder_->OpenFile(file_path);
        vb_decoder_->GetFormat(rate, channels);
        vb_decoder_->SetOutputFormat(rate, channels);

        output_decoder_ = std::make_unique<MP3Decoder>();

        output_decoder_->OpenFile(file_path);
        output_decoder_->SetOutputFormat(rate, channels);
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

    int output_device_index = Pa_GetDefaultOutputDevice();
    PaStreamParameters output_parameters2 =
        {
            .device = output_device_index,
            .channelCount = channels,
            .sampleFormat = paInt16,
            .suggestedLatency = Pa_GetDeviceInfo(output_device_index)->defaultLowOutputLatency,
            .hostApiSpecificStreamInfo = nullptr};

    const PaError pa_format_err = Pa_IsFormatSupported(nullptr, &output_parameters, rate);
    if (pa_format_err != paNoError)
    {
        throw std::runtime_error("The format is not supported. Error: "s + std::string(Pa_GetErrorText(pa_format_err)));
    }

    const unsigned long FRAMES_PER_BUFFER = 1024;

    const PaError vb_pa_open_err = Pa_OpenStream(
        &vb_stream_,
        nullptr,
        &output_parameters,
        rate,
        FRAMES_PER_BUFFER,
        paClipOff,
        &SoundPlayer::VBPaCallback,
        this);

    const PaError out_pa_open_err = Pa_OpenStream(
        &output_stream_,
        nullptr,
        &output_parameters2,
        rate,
        FRAMES_PER_BUFFER,
        paClipOff,
        &SoundPlayer::OutputPaCallback,
        this);

    if (vb_pa_open_err != paNoError || out_pa_open_err != paNoError)
    {
        throw std::runtime_error("Failed to open audio stream."s);
    }
}

int SoundPlayer::VBPaCallback(const void *inputBuffer,
                              void *outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo *,
                              PaStreamCallbackFlags,
                              void *userData)
{
    auto *player = static_cast<SoundPlayer *>(userData);
    short *out = static_cast<short *>(outputBuffer);
    size_t samples_needed = framesPerBuffer * player->channels_;

    size_t bytes_read = 0;
    int err = player->vb_decoder_->ReadFile(
        reinterpret_cast<unsigned char *>(out),
        samples_needed * sizeof(short),
        bytes_read);

    float volume;
    {
        std::lock_guard<std::mutex> lock(player->vb_volume_mutex_);
        volume = player->vb_volume_;
    }

    size_t samples_read = bytes_read / sizeof(short);
    player->ChangeVolume(out, samples_read, volume);

    if (err == MPG123_DONE)
    {
        std::memset(out + samples_read, 0,
                    (samples_needed - samples_read) * sizeof(short));
        return paComplete;
    }

    return paContinue;
}

int SoundPlayer::OutputPaCallback(const void *inputBuffer,
                                  void *outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo *,
                                  PaStreamCallbackFlags,
                                  void *userData)
{
    auto *player = static_cast<SoundPlayer *>(userData);
    short *out = static_cast<short *>(outputBuffer);
    size_t samples_needed = framesPerBuffer * player->channels_;

    size_t bytes_read = 0;
    int err = player->output_decoder_->ReadFile(
        reinterpret_cast<unsigned char *>(out),
        samples_needed * sizeof(short),
        bytes_read);

    float volume;
    {
        std::lock_guard<std::mutex> lock(player->out_volume_mutex_);
        volume = player->output_volume_;
    }

    size_t samples_read = bytes_read / sizeof(short);
    player->ChangeVolume(out, samples_read, volume);

    if (err == MPG123_DONE)
    {
        std::memset(out + samples_read, 0,
                    (samples_needed - samples_read) * sizeof(short));
        return paComplete;
    }

    return paContinue;
}

void SoundPlayer::ChangeVolume(short *buffer, size_t num_samples, float volume)
{
    for (size_t i = 0; i < num_samples; ++i)
    {
        float scaled = static_cast<float>(buffer[i]) * volume;
        if (scaled > 32767.0f)
        {
            scaled = 32767.0f;
        }
        else if (scaled < -32768.0f)
        {
            scaled = -32768.0f;
        }
        buffer[i] = static_cast<short>(scaled);
    }
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