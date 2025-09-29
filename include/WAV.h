#ifndef _converter_WAV_H_INCLUDED
#define _converter_WAV_H_INCLUDED

#include "file_data.h"

namespace wav
{
    audio_data read(std::string filename);
    void write(std::string filename, audio_data* data);
} // namespace bmp

#endif