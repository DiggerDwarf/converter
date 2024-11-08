#ifndef _converter_file_data_h_INCLUDED
#define _converter_file_data_h_INCLUDED

#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cmath>

typedef unsigned char BYTE;

bool equals(const char* str1, const char* str2);
void trim(std::string& str);

extern bool verbose;

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
    IMAGE
} file_type;

typedef struct __pixel {
    BYTE r;
    BYTE g;
    BYTE b;
} pixel;

typedef struct __image_data {
    int height;
    int width;
    pixel **pixels;

    __image_data()
    {
        this->height = 0;
        this->width = 0;
        this->pixels = nullptr;
    }

    __image_data(int height, int width, pixel **pixels)
    {
        this->height = height;
        this->width = width;
        this->pixels = pixels;
    }

    ~__image_data() {
        for (int i = 0; i < this->height; i++)
        {
            delete this->pixels[i];
        }
        delete this->pixels;
    }

} image_data;

#endif