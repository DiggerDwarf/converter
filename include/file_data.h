#ifndef _converter_file_data_h_INCLUDED
#define _converter_file_data_h_INCLUDED

#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cmath>
#include <stdint.h>

bool equals(const char* str1, const char* str2);
void trim(std::string& str);


struct File     // File convenice
{
private:
    FILE* file;             // Actual file object
    char current;           // Current char in file at pos
    fpos_t pos;             // Position in file
    bool isOpen = false;    // Is a file open in this struct
public:
    // Advance in the file
    File& operator++()
    {
        if (!this->isOpen) return *this;
        pos++;
        fsetpos(this->file, &this->pos);
        if ((this->current = fgetc(this->file)) == EOF)
        {
            this->current = '\0';
        }
        fsetpos(this->file, &this->pos);
        return *this;
    }
    File  operator++(int)
    {
        return ++*this;
    }
    // Go back in the file
    File& operator--()
    {
        if (!this->isOpen) return *this;
        pos--;
        fsetpos(this->file, &this->pos);
        if ((this->current = fgetc(this->file)) == EOF)
        {
            this->current = '\0';
        }
        fsetpos(this->file, &this->pos);
        return *this;
    }
    File  operator--(int)
    {
        return --*this;
    }
    // Fetch current character
    char  operator()()
    {
        return this->isOpen ? this->current : '\0';
    }
    // Open a file
    void open(const char* fileName)
    {
        this->isOpen = true;
        this->file = fopen(fileName, "rb");
        fgetpos(this->file, &this->pos);
        this->current = fgetc(this->file);
        fsetpos(this->file, &this->pos);
    }
    // Close the file
    void close()
    {
        this->isOpen = false;
        fclose(this->file);
    }
};

typedef enum {
    IMAGE,
    AUDIO
} file_type;

typedef struct __pixel {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;
} pixel;

bool operator==(pixel p1, pixel p2);

std::ostream& operator<<(std::ostream& stream, pixel pix);

typedef struct __image_data {
    pixel *pixels;
    int height;
    int width;

    __image_data()
    {
        this->height = 0;
        this->width = 0;
        this->pixels = nullptr;
    }

    __image_data(int height, int width)
    {
        this->height = height;
        this->width = width;
        this->pixels = new pixel[height * width];
    }

    ~__image_data()
    {
        delete[] this->pixels;
    }

} image_data;

typedef struct __audio_data {
    // 8 Byte data
    union {
        std::uint8_t** uint8_samples;
        std::int16_t** int16_samples;
        std::int32_t** int32_samples;
        float** float32_samples;
    };

    // 4 Byte data
    std::uint32_t numSamples;
    std::uint32_t sampleRate;
    std::uint32_t bytesPerSample;

    // 2 Byte data
    std::uint16_t numChannels;

    ~__audio_data() {
        switch (this->bytesPerSample)
        {
        case 1:
            delete this->uint8_samples;
            break;
        case 2:
            delete this->int16_samples;
            break;
        case 3:
            delete this->int32_samples;
            break;
        case 4:
            delete this->float32_samples;
            break;
        
        default:
            break;
        }
    }
} audio_data;

#endif