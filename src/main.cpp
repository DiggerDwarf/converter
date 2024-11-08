#include <iostream>
#include <map>

#include "file_data.h"

bool equals(const char* str1, const char* str2)
{
    int i = 0;
    while (str1[i] != '\0' && str1[i] == str2[i]) ++i;
    return str1[i] == '\0' && str2[i] == '\0';
}

void trim(std::string &str)
{
    size_t start = str.find_first_not_of(' ');
    if (start == std::string::npos)
    {
        str = "";
        return;
    }
    size_t end = str.find_last_not_of(' ');
    str = str.substr(start, end - start + 1);
}

#include "BMP.h"
#include "PNG.h"

std::map<std::string, file_type> EXTENSION_TO_TYPE = {
    {"bmp", file_type::IMAGE},
    {"png", file_type::IMAGE},
};

bool verbose = false;

int main(int argc, char const *argv[])
{
    std::string sourceFile;

    for (int i = 1; i < argc; i++)
    {
        if (equals(argv[i], "--verbose") || equals(argv[i], "-v")) {
            verbose = true;
        }
        else if (equals(argv[i], "--file")) {
            if (i < argc - 1) {
                sourceFile = argv[++i];
                if (not std::filesystem::exists(sourceFile.c_str())) {
                    printf("Target file %s not found\n", sourceFile.c_str());
                    exit(3);
                }
            }
            else {
                puts("Expected argument after --file flag\n");
                exit(3);
            }
        }
    }

    if (sourceFile.empty())
    {
        puts("No input file was given, terminating.\n");
        exit(0);
    }

    image_data a = bmp::read(sourceFile.c_str());

    std::cout << (int)a.pixels[0][0].r << (int)a.pixels[0][0].g << (int)a.pixels[0][0].b << std::endl;

    bmp::write(sourceFile+".test.bmp", a);

    return 0;
}