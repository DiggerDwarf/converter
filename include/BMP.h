#ifndef _converter_BMP_H_INCLUDED
#define _converter_BMP_H_INCLUDED

#include "file_data.h"

namespace bmp
{
    image_data read(std::string filename);
    void write(std::string filename, image_data& data);
} // namespace bmp

#endif