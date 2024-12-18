#include <iostream>
#include <map>

#include "file_data.h"
#include "BMP.h"
#include "PNG.h"
#include "QOI.h"

bool operator==(pixel p1, pixel p2)
{
    return p1.r == p2.r && p1.g == p2.g && p1.b == p2.b && p1.a == p2.a; 
}

std::ostream& operator<<(std::ostream& stream, pixel pix)
{
    return stream << '<' << pix.r << ' ' << pix.g << ' ' << pix.b << '>';
}

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

// Map extension names to data type
std::map<std::string, file_type> EXTENSION_TO_TYPE = {
    {"bmp", file_type::IMAGE},
    {"png", file_type::IMAGE},
    {"qoi", file_type::IMAGE}
};

// Handle calling the right function to read image data
image_data read_image(std::string path, std::string ext)
{
    if (ext == "bmp") {
        return bmp::read(path);
    } else if (ext == "png") {
        return png::read(path);
    } else if (ext == "qoi") {
        return qoi::read(path);
    }
    else {
        std::puts("Unknown extension (you're not supposed to be able to output that but ok i guess)");
        std::exit(42);
    }
}

// Handle calling the right function to write image data
void write_image(std::string path, std::string ext, image_data* image)
{
    if (ext == "bmp") {
        bmp::write(path, image);
    } else if (ext == "png") {
        png::write(path, image);
    } else if (ext == "qoi") {
        qoi::write(path, image);
    }
    else {
        std::puts("Unknown extension (you're not supposed to be able to output that but ok i guess)");
        std::exit(42);
    }
}

// Declare global variable verbose to be reused by everyone
bool verbose = false;

int main(int argc, char const *argv[])
{
    std::string sourceFile;
    std::string outputFile;

    for (int i = 1; i < argc; i++)      // Read arguments passed to the program
    {
        if (equals(argv[i], "--help") || equals(argv[i], "-h")) {   // If help flag is found, display help and quit
            std::puts("Command line file format converter");
            std::puts("Options:");
            std::puts("  -h, --help                  Display this message and exit");
            std::puts("  -f [path], --file [path]    Source file to be converted");
            std::puts("  -o [path], --output [path]  Path to output file");
            std::puts("  -v, --verbose               Describe the steps taken to convert the file");
            std::exit(0);
        }
        else if (equals(argv[i], "--verbose") || equals(argv[i], "-v")) {   // If verbose flag is found, enable verbose
            verbose = true;
        }
        else if (equals(argv[i], "--file") || equals(argv[i], "-f")) {      // if file flag is found, set source file path
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
        else if (equals(argv[i], "--output") || equals(argv[i], "-o")) {    // if output flag is found, set output file path
            if (i < argc - 1) {
                outputFile = argv[++i];
            }
            else {
                std::puts("Expected argument after --output flag");
                std::exit(3);
            }
        }
    }

    #pragma region Parse I/O names

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
        std::printf("Source extension \"%s\" not implemented yet\n", sourceExt.c_str());
        std::exit(3);
    } else if (EXTENSION_TO_TYPE.count(outputExt) == 0) {
        std::printf("Output extension \"%s\" not implemented yet\n", outputExt.c_str());
        std::exit(3);
    }

    // Check if those formats are compatible
    if (EXTENSION_TO_TYPE.at(sourceExt) != EXTENSION_TO_TYPE.at(outputExt)) {
        std::printf("Input and output formats (\"%s\", \"%s\") are incompatible.\n", sourceExt.c_str(), outputExt.c_str());
        std::exit(3);
    }

    #pragma endregion

    switch (EXTENSION_TO_TYPE.at(sourceExt))    // Switch over data types
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