#include <iostream>
#include <map>

#include "file_data.h"
#include "BMP.h"
#include "PNG.h"

bool equals(const char* str1, const char* str2)
{
    int i = 0;
    while (str1[i] != '\0' && str1[i] == str2[i]) ++i;
    return str1[i] == '\0' && str2[i] == '\0';
}

void trim(std::string &str)
{
    std::size_t start = str.find_first_not_of(' ');
    if (start == std::string::npos)
    {
        str = "";
        return;
    }
    std::size_t end = str.find_last_not_of(' ');
    str = str.substr(start, end - start + 1);
}

std::map<std::string, file_type> EXTENSION_TO_TYPE = {
    {"bmp", file_type::IMAGE},
    {"png", file_type::IMAGE}
};

std::map<std::string, int> EXT_ID = {
    {"bmp", 0},
    {"png", 1}
};

image_data read_image(std::string path, std::string ext)
{
    if (ext == "bmp") {
        return bmp::read(path);
    } else if (ext == "png") {
        return png::read(path);
    }
    else {
        std::puts("Unknown extension (you're not supposed to be able to output that but ok i guess)");
        std::exit(42);
    }
}

void write_image(std::string path, std::string ext, image_data* image)
{
    if (ext == "bmp") {
        bmp::write(path, image);
    } else if (ext == "png") {
        png::write(path, image);
    }
    else {
        std::puts("Unknown extension (you're not supposed to be able to output that but ok i guess)");
        std::exit(42);
    }
}

bool verbose = false;

int main(int argc, char const *argv[])
{
    std::string sourceFile;
    std::string outputFile;

    for (int i = 1; i < argc; i++)      // Read arguments passed to the program
    {
        if (equals(argv[i], "--help") || equals(argv[i], "-h")) {
            std::puts("Command line file format converter");
            std::puts("Options:");
            std::puts("  -h, --help                  Display this message and exit");
            std::puts("  -f [path], --file [path]    Source file to be converted");
            std::puts("  -o [path], --output [path]  Path to output file");
            std::puts("  -v, --verbose               Describe the steps taken to convert the file");
            std::exit(0);
        }
        else if (equals(argv[i], "--verbose") || equals(argv[i], "-v")) {
            verbose = true;
        }
        else if (equals(argv[i], "--file") || equals(argv[i], "-f")) {
            if (i < argc - 1) {
                sourceFile = argv[++i];
                if (not std::filesystem::exists(sourceFile.c_str())) {
                    printf("Target file %s not found\n", sourceFile.c_str());
                    std::exit(3);
                }
            }
            else {
                std::puts("Expected argument after --file flag");
                std::exit(3);
            }
        }
        else if (equals(argv[i], "--output") || equals(argv[i], "-o")) {
            if (i < argc - 1) {
                outputFile = argv[++i];
            }
            else {
                std::puts("Expected argument after --output flag");
                std::exit(3);
            }
        }
    }

    if (sourceFile.empty()) {           // Check for target existence
        std::puts("No input file was given, terminating.");
        std::exit(0);
    }
    if (outputFile.empty()) {           // Check if output path is given and generate one otherwise
        std::size_t pos = sourceFile.find_last_of('/');
        if (pos == std::string::npos) {
            outputFile = "new_" + sourceFile;
        }
        else {
            outputFile = sourceFile.substr(0, pos+1) + "new_" + sourceFile.substr(pos+1);
        }
    }

    // Find source and ouput formats
    std::size_t whereDot = sourceFile.find_last_of('.');
    if (whereDot == std::string::npos) {
        std::puts("Could not deduce source format from file name.");
        std::exit(3);
    }
    std::string sourceExt = sourceFile.substr(whereDot+1);
    whereDot = outputFile.find_last_of('.');
    if (whereDot == std::string::npos) {
        std::puts("Could not deduce output format from file name.");
        std::exit(3);
    }
    std::string outputExt = outputFile.substr(whereDot+1);

    // Check if these formats are implemented yet
    if (EXTENSION_TO_TYPE.count(sourceExt) == 0) {
        std::puts("Source extension not implemented yet");
        std::exit(3);
    } else if (EXTENSION_TO_TYPE.count(outputExt) == 0) {
        std::puts("Output extension not implemented yet");
        std::exit(3);
    }

    // Check if those formats are compatible
    if (EXTENSION_TO_TYPE.at(sourceExt) != EXTENSION_TO_TYPE.at(outputExt)) {
        std::puts("Input and output format are incompatible.");
        std::exit(3);
    }

    switch (EXTENSION_TO_TYPE[sourceExt])
    {
        case file_type::IMAGE: {
            image_data image = read_image(sourceFile, sourceExt);
            write_image(outputFile, outputExt, &image);
            break;
        }
        default:
            break;
    }

    return 0;
}