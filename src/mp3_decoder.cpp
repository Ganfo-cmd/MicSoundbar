#include "mp3_decoder.h"

#include <stdexcept>

using namespace std::string_literals;

MP3Decoder::MP3Decoder()
{
    int err = MPG123_OK;
    mpg_handler_ = mpg123_new(NULL, &err);
    if (mpg_handler_ == nullptr)
    {
        throw std::runtime_error("Failed to create mpg123_handle. Error: "s + std::string(mpg123_plain_strerror(err)));
    }
}

MP3Decoder::~MP3Decoder()
{
    CloseDecoder();
}

void MP3Decoder::CloseDecoder()
{
    if (mpg_handler_)
    {
        mpg123_close(mpg_handler_);
        mpg123_delete(mpg_handler_);
        mpg_handler_ = nullptr;
    }
}

void MP3Decoder::OpenFile(const char *file_path)
{
    if (mpg123_open(mpg_handler_, file_path) != MPG123_OK)
    {
        throw std::runtime_error("Failed to open mp3 file. Error: "s + std::string(mpg123_strerror(mpg_handler_)));
    }
}

void MP3Decoder::GetFormat(long &rate, int &channels)
{
    if (mpg123_getformat(mpg_handler_, &rate, &channels, &encoding_) != MPG123_OK)
    {
        throw std::runtime_error("Failed to get audio format. Error: "s + std::string(mpg123_strerror(mpg_handler_)));
    }
}

void MP3Decoder::SetOutputFormat(long rate, int channels)
{
    encoding_ = MPG123_ENC_SIGNED_16;
    if (mpg123_format(mpg_handler_, rate, channels, encoding_) != MPG123_OK)
    {
        throw std::runtime_error("Failed to set output format. Error: "s + std::string(mpg123_strerror(mpg_handler_)));
    }
}

size_t MP3Decoder::GetOutblockSize() const
{
    return mpg123_outblock(mpg_handler_);
}

int MP3Decoder::ReadFile(unsigned char *buffer, size_t buffer_size, size_t &done)
{
    int err = MPG123_OK;
    err = mpg123_read(mpg_handler_, buffer, buffer_size, &done);

    return err;
}

std::string MP3Decoder::GetErrorText() const
{
    return std::string(mpg123_strerror(mpg_handler_));
}