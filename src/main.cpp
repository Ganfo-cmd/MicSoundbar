#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

// внешние библиотеки
#include <mpg123.h>
#include <portaudio.h>

template <typename T>
void FreeBuffer(T *&ptr)
{
    if (ptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

// Освобождение handle
void CleanUp(mpg123_handle *&mh, unsigned char *&buffer)
{
    if (mh)
    {
        mpg123_close(mh);
        mpg123_delete(mh);
        mh = nullptr;
    }

    FreeBuffer(buffer);
}

int main()
{
    mpg123_handle *mh = nullptr;
    unsigned char *buffer = nullptr;
    size_t buffer_size = 0;
    int channels = 0;
    int encoding = 0;
    long rate = 0;
    int err = MPG123_OK;

    // Создание дескриптора. Указываем NULL для выбора дескриптора по умолчанию
    mh = mpg123_new(NULL, &err);
    if (mh == nullptr)
    {
        std::cerr << "Failed to create mpg123_handle. Error: " << mpg123_plain_strerror(err) << std::endl;
        return 1;
    }

    // Открытие MP3-файла, пока просто тестирование, можно заменить на любой mp3 файл
    if (mpg123_open(mh, "C:/file.mp3") != MPG123_OK)
    {
        std::cerr << "Failed to open mp3 file. Error: " << mpg123_strerror(mh) << std::endl;
        return 1;
    }

    // Получение информации о формате
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK)
    {
        std::cerr << "Failed to get audio format. Error: " << mpg123_strerror(mh) << std::endl;
        CleanUp(mh, buffer);
        return 1;
    }

    // Устанавливается стандартный формат 16-битный PCM
    encoding = MPG123_ENC_SIGNED_16;
    if (mpg123_format(mh, rate, channels, encoding) != MPG123_OK)
    {
        std::cerr << "Failed to set output format. Error: " << mpg123_strerror(mh) << std::endl;
        CleanUp(mh, buffer);
        return 1;
    }

    std::cout << "Audio info: " << rate << " Hz, " << channels << " channels" << std::endl;

    buffer_size = mpg123_outblock(mh);
    buffer = new unsigned char[buffer_size];

    // Инициализация PortAudio
    const PaError pa_init_err = Pa_Initialize();
    if (pa_init_err != paNoError)
    {
        std::cerr << "PortAudio initialization failed. Error: " << Pa_GetErrorText(pa_init_err) << std::endl;
        CleanUp(mh, buffer);
        return 1;
    }

    // Поиск необходимого устройства Input Cabel (VB-Cabel)
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
        std::cerr << "CABLE Input device not found!" << std::endl;
        Pa_Terminate();
        CleanUp(mh, buffer);
        return 1;
    }

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
        std::cerr << "The format is not supported. Error: " << Pa_GetErrorText(pa_format_err) << std::endl;
        Pa_Terminate();
        CleanUp(mh, buffer);
        return 1;
    }

    PaStream *stream = nullptr;
    const PaError pa_open_err = Pa_OpenStream(
        &stream,
        nullptr,
        &output_parameters,
        rate,
        paFramesPerBufferUnspecified,
        paClipOff,
        nullptr,
        nullptr);

    if (pa_open_err != paNoError)
    {
        std::cerr << "Failed to open audio stream. Error: " << Pa_GetErrorText(pa_open_err) << std::endl;
        Pa_Terminate();
        CleanUp(mh, buffer);
        return 1;
    }

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5s);

    const PaError pa_start_err = Pa_StartStream(stream);
    if (pa_start_err != paNoError)
    {
        std::cerr << "Failed to start audio stream. Error: " << Pa_GetErrorText(pa_start_err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        CleanUp(mh, buffer);
    }

    std::cout << "Streaming audio..." << std::endl;

    // Основной цикл воспроизведения
    size_t done = 0;
    PaError pa_write_err;
    while ((err = mpg123_read(mh, buffer, buffer_size, &done)) == MPG123_OK)
    {
        // Если mpg123 прочитал 0 байт
        if (done == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        pa_write_err = Pa_WriteStream(stream, buffer, done / (channels * sizeof(short)));
        if (pa_write_err != paNoError)
        {
            std::cerr << "Failed to write audio. Error: " << Pa_GetErrorText(pa_write_err) << std::endl;
            break;
        }
    }

    if (err != MPG123_DONE)
    {
        std::cerr << "MP3 decoding error. Error: " << mpg123_strerror(mh) << std::endl;
    }

    std::cout << "Playback finished" << std::endl;
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    CleanUp(mh, buffer);
    return 0;
}