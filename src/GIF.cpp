// #include "GIF.h"

// image_data gif::read(std::string filename)
// {
//     if (verbose) std::puts("Not implemented");
//     std::exit(4);

//     FILE* file = std::fopen(filename.c_str(), "rb");

//     char signature[7] = {};
//     std::fread(&signature, 1, 6, file);
//     if (not equals(signature, "GIF89a")) { // Check BM signature
//         if (verbose) std::printf("Invalid file signature ( %s ) vs expected for gif files ( GIF89a )\n", signature);
//         std::fclose(file);
//         std::exit(1);
//     }

//     image_data image;

//     uint16_t width, height;
//     uint8_t GCT, backgroundColor, aspectRatio;

//     std::fread(&width, 2, 1, file);
//     std::fread(&height, 2, 1, file);
//     std::fread(&GCT, 1, 1 , file);
//     std::fread(&backgroundColor, 1, 1 , file);
//     std::fread(&aspectRatio, 1, 1 , file);

//     bool GCTFollowsDescriptor = ((GCT & 0b10000000) >> 7) == 1;
//     int bitsperColor = ((GCT & 0b01110000) >> 4) + 1;
//     int bitsPerPixel = ((GCT & 0b00000111) >> 0) + 1;

//     if (bitsperColor != 8) {
//         if (verbose) std::puts("Fuck you");
//         std::fclose(file);
//         std::exit(43);
//     }

//     pixel palette[1 << bitsperColor];

//     for (int i = 0; i < (1 << bitsPerPixel); i++)
//     {
//         std::fread(&palette[i], 3, 1, file);
//     }
    
//     uint8_t miniSignature;
//     std::fread(&miniSignature, 1, 1, file);
//     if (miniSignature != '!') {
//         std::puts("Supposed to find a Graphic Control Extension but didn't");
//         std::fclose(file);
//         std::exit(42);
//     }

//     std::fclose(file);

//     return image_data();
// }

// void gif::write(std::string filename, image_data *data)
// {
//     if (verbose) std::puts("Not implemented");
//     std::exit(4);
// }
