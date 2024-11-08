#include "BMP.h"

typedef unsigned int uint;

image_data bmp::read(std::string filename)
{
    FILE* file = std::fopen(filename.c_str(), "rb");
    
    char signature[3] = {};
    std::fread(&signature, 1, 2, file);
    if (not equals(signature, "BM")) { // Check BM signature
        if (verbose) std::printf("Invalid file signature ( %s ) vs expected for bmp files ( BM )\n", signature);
        std::fclose(file);
        std::exit(1);
    }

    image_data image;

    #pragma region Header variables

    if (verbose) puts("Reading header . . .");

    uint fileSize, pixelArrayStart, dibHeaderSize, compressionType, pixelArraySize, colorsInPalette, nbImportantColors;
    char reserved[5] = {};
    short nbColorPlanes, bitsPerPixel;
    int horizontalRes, verticalRes, padding;

    std::fread(&fileSize, 4, 1, file);
    std::fread(&(reserved[0]), 4, 1, file);
    
    if (equals(reserved, "dWRF") && verbose) puts("Looks like this file was already made by me...");
    std::fread(&pixelArrayStart, 4, 1, file);
    std::fread(&dibHeaderSize, 4, 1, file);
    if (dibHeaderSize != 40) { // Check header type is BITMAPINFOHEADER
        if (verbose) std::puts("BMP files with this header are sadly not yet supported...");
        std::fclose(file);
        std::exit(2);
    }

    std::fread(&image.width, 4, 1, file);
    std::fread(&image.height, 4, 1, file);
    std::fread(&nbColorPlanes, 2, 1, file);
    std::fread(&bitsPerPixel, 2, 1, file);
    std::fread(&compressionType, 4, 1, file);
    if (compressionType != 0 || bitsPerPixel != 24) {
        if (verbose) std::puts("BMP files with this compression type are sadly not yet supported...");
        std::fclose(file);
        std::exit(2);
    }
    std::fread(&pixelArraySize, 4, 1, file);
    if (pixelArraySize == 0) {
        pixelArraySize = (image.width*image.height*bitsPerPixel) >> 3;
    }
    std::fread(&horizontalRes, 4, 1, file);
    std::fread(&verticalRes, 4, 1, file);
    std::fread(&colorsInPalette, 4, 1, file);
    if (colorsInPalette == 0) {
        colorsInPalette = 1 << bitsPerPixel;
    }
    std::fread(&nbImportantColors, 4, 1, file);
    padding = (int)std::ceil((image.width*bitsPerPixel)/8.0) % 4;
    
    #pragma endregion Header variables

    #pragma region Image data

    if (verbose) puts("Reading data . . .");

    // Align to start of the pixel array
    std::fseek(file, pixelArrayStart - (14 + 40), SEEK_CUR);
    
    image.pixels = new pixel*[image.height];


    for (int i = 0; i < image.height; i++)
    {
        image.pixels[i] = new pixel[image.width];

        std::fread(image.pixels[i], 3, image.width, file);
        std::fseek(file, padding, SEEK_CUR);
    }

    #pragma endregion Image data

    std::fclose(file);

    if (verbose) puts("Finished reading file !");

    return image;
}

void bmp::write(std::string filename, image_data* data)
{
    if (std::filesystem::exists(filename.c_str()))
    {
        std::cout << "The file \"" << filename << "\" already exists. Do you want to overwrite it ? [ y / .. ] ";
        std::string input; std::getline(std::cin, input); trim(input);
        if (input != "y" && input != "Y")
        {
            std::puts("User cancel; aborting operation");
            return;
        }
    }

    FILE* file = fopen(filename.c_str(), "wb");

    // We'll use BITMAPINFOHEADER for the dib header and BI_RGB for the compression

    uint fileSize = 0x36 + data->height*data->width*3,
        pixelArrayStart = 0x36,
        dibHeaderSize = 0x28,
        compressionType = 0,
        pixelArraySize = 0,
        colorsInPalette = 0,
        nbImportantColors = 0;
    short nbColorPlanes = 1,
        bitsPerPixel = 24;
    int horizontalRes = 0,
        verticalRes = 0,
        padding = (data->width*3) % 4;
    const char* signature = "BM";

    char padder[padding];
    memset(&padder, 0, padding);

    const char reserved[5] = "dWRF";

    if (verbose) puts("Writing header . . .");

    std::fwrite(signature, 1, 2, file);
    std::fwrite(&fileSize, 4, 1, file);
    std::fwrite(&reserved, 1, 4, file);
    std::fwrite(&pixelArrayStart, 4, 1, file);
    std::fwrite(&dibHeaderSize, 4, 1, file);
    std::fwrite(&data->width, 4, 1, file);
    std::fwrite(&data->height, 4, 1, file);
    std::fwrite(&nbColorPlanes, 2, 1, file);
    std::fwrite(&bitsPerPixel, 2, 1, file);
    std::fwrite(&compressionType, 4, 1, file);
    std::fwrite(&pixelArraySize, 4, 1, file);
    std::fwrite(&horizontalRes, 4, 1, file);
    std::fwrite(&verticalRes, 4, 1, file);
    std::fwrite(&colorsInPalette, 4, 1, file);
    std::fwrite(&nbImportantColors, 4, 1, file);

    if (verbose) puts("Writing data . . .");

    for (int i = 0; i < data->height; i++)
    {
        fwrite(data->pixels[i], 3, data->width, file);
        fwrite(padder, 1, padding, file);
    }

    if (verbose) puts("Finished writing file !");

    std::fclose(file);
}
