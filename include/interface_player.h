#pragma once
#include <string>

class AudioInterfacePlayer
{
public:
    virtual ~AudioInterfacePlayer() = default;
    virtual void Play(const std::string &file_path) = 0;
    virtual void Stop() = 0;
    virtual void SetVolume() = 0;
    virtual bool IsPlaying() const = 0;
};
