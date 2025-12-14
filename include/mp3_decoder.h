#pragma once

#include <mpg123.h>
#include <string>

class MP3Decoder
{
public:
    MP3Decoder();
    ~MP3Decoder();

    void CloseDecoder();

    void OpenFile(const char *file_path);

    void GetFormat(long &rate, int &channels);

    void SetOutputFormat(long rate, int channels);

    int ReadFile(unsigned char *buffer, size_t buffer_size, size_t &done);

    size_t GetOutblockSize() const;

    std::string GetErrorText() const;

private:
    int encoding_ = 0;
    mpg123_handle *mpg_handler_ = nullptr;
};