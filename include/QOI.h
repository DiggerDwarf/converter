#ifndef _converter_QOI_H_INCLUDED
#define _converter_QOI_H_INCLUDED

#include "file_data.h"

namespace qoi
{
    image_data read(std::string filename);
    void write(std::string filename, image_data* data);
} // namespace qoi

#endif