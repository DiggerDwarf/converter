#ifndef _converter_PNG_H_INCLUDED
#define _converter_PNG_H_INCLUDED

#include "file_data.h"

namespace png
{
    image_data read(std::string filename);
    void write(std::string filename, image_data* data);
} // namespace png


#endif