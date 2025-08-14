#include <iostream>
#include <mpg123.h>
#include <portaudio.h>
#include <cstring>
#include <thread>
#include <chrono>

template <typename T>
void FreePtr(T *&ptr)
{
    if (ptr)
    {
        delete[] ptr;
        ptr = nullptr;
    }
}

int main()
{
    // Инициализация mpg123
    mpg123_handle *mh = nullptr;
    unsigned char *buffer = nullptr;
    size_t buffer_size = 0;
    int err = 0;
    int channels = 0, encoding = 0;
    long rate = 0;

    // Инициализация библиотеки mpg123
    if (mpg123_init() != MPG123_OK)
    {
        std::cerr << "Failed to initialize mpg123 library" << std::endl;
        return 1;
    }

    // Создание дескриптора декодера
    mh = mpg123_new(nullptr, &err);
    if (!mh)
    {
        std::cerr << "Failed to create mpg123 handle: " << mpg123_plain_strerror(err) << std::endl;
        mpg123_exit();
        return 1;
    }

    // Открытие MP3-файла, пока просто тестирование, можно заменить на любой mp3 файл
    if (mpg123_open(mh, "C:/file.mp3") != MPG123_OK)
    {
        std::cerr << "Failed to open MP3 file: " << mpg123_strerror(mh) << std::endl;
        mpg123_delete(mh);
        mpg123_exit();
        return 1;
    }

    // Получение информации о формате аудио
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK)
    {
        std::cerr << "Failed to get audio format" << std::endl;
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return 1;
    }

    
    encoding = MPG123_ENC_SIGNED_16;
    if (mpg123_format(mh, rate, channels, encoding) != MPG123_OK)
    {
        std::cerr << "Failed to set output format" << std::endl;
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return 1;
    }

    std::cout << "Audio info: " << rate << " Hz, " << channels << " channels" << std::endl;

    buffer_size = mpg123_outblock(mh);
    buffer = new unsigned char[buffer_size];

    // Инициализация PortAudio
    PaError paErr = Pa_Initialize();
    if (paErr != paNoError)
    {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(paErr) << std::endl;
        FreePtr(buffer);
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return 1;
    }

    // Поиск устройства VB-Cable
    int vbCableIndex = -1;
    int numDevices = Pa_GetDeviceCount();
    const char *targetDeviceName = "CABLE Input";

    for (int i = 0; i < numDevices; i++)
    {
        const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo->maxOutputChannels > 0)
        {
            std::cout << "Found audio device #" << i << ": " << deviceInfo->name << std::endl;
            if (strstr(deviceInfo->name, targetDeviceName) != nullptr)
            {
                vbCableIndex = i;
                break;
            }
        }
    }

    if (vbCableIndex == -1)
    {
        std::cerr << "VB-Cable output device not found!" << std::endl;
        Pa_Terminate();
        FreePtr(buffer);
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return 1;
    }

    std::cout << "Using VB-Cable device at index: " << vbCableIndex << std::endl;

    // Настройка параметров вывода
    PaStreamParameters outputParameters;
    outputParameters.device = vbCableIndex;
    outputParameters.channelCount = channels;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(vbCableIndex)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    PaStream *stream = nullptr;
    paErr = Pa_OpenStream(
        &stream,
        nullptr,                      // Нет входного потока
        &outputParameters,            // Параметры вывода
        rate,                         // Частота дискретизации
        paFramesPerBufferUnspecified, // Автоматический размер буфера
        paClipOff,                    // Отключаем клиппинг
        nullptr,                      // Нет callback-функции
        nullptr                       // Нет пользовательских данных
    );

    if (paErr != paNoError)
    {
        std::cerr << "Failed to open audio stream: " << Pa_GetErrorText(paErr) << std::endl;
        Pa_Terminate();
        FreePtr(buffer);
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return 1;
    }

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5s);
	
    // Запуск аудиопотока
    paErr = Pa_StartStream(stream);
    if (paErr != paNoError)
    {
        std::cerr << "Failed to start audio stream: " << Pa_GetErrorText(paErr) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        FreePtr(buffer);
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return 1;
    }

    std::cout << "Streaming audio to VB-Cable..." << std::endl;

    // Основной цикл воспроизведения
    size_t done = 0;
    while ((err = mpg123_read(mh, buffer, buffer_size, &done)) == MPG123_OK)
    {
        if (done == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        paErr = Pa_WriteStream(stream, buffer, done / (channels * sizeof(short)));
        if (paErr != paNoError)
        {
            std::cerr << "Audio write error: " << Pa_GetErrorText(paErr) << std::endl;
            break;
        }
    }

    if (err != MPG123_DONE)
    {
        std::cerr << "MP3 decoding error: " << mpg123_strerror(mh) << std::endl;
    }

    // Завершение воспроизведения
    std::cout << "Playback finished" << std::endl;
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    FreePtr(buffer);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    return 0;
}
