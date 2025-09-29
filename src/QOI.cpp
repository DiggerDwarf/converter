#include "QOI.h"

#define QOI_HASH(pix) (((3*pix.r) + (5*pix.g) + (7*pix.b) + (11*pix.a)) & 0b111111)
#define QOI_OP_RGB   0b11111110
#define QOI_OP_RGBA  0b11111111
#define QOI_OP_INDEX 0b00000000
#define QOI_OP_DIFF  0b01000000
#define QOI_OP_LUMA  0b10000000
#define QOI_OP_RUN   0b11000000
// #define MIN_WRAP(a)  (__min(a % 256, (-a) % 256))
#define MIN_WRAP(a)  ((abs(a % 256) < abs((-a) % 256)) ? (a % 256) : ((-a) % 256))


image_data qoi::read(std::string filename)
{
    FILE* file = std::fopen(filename.c_str(), "rb");

    char signature[5] = {};
    std::fread(&signature, 1, 4, file);
    if (not equals(signature, "qoif")) { // Check qoif signature
        if (verbose) std::printf("Invalid file signature ( %s ) vs expected for qoi files ( qoif )\n", signature);
        std::fclose(file);
        std::exit(1);
    }

    image_data image;

    if (verbose) std::puts("Reading header . . .");

    char width[4], height[4];
    uint8_t channels, colorspace;

    std::fread(&width, 1, 4, file);
    std::fread(&height, 1, 4, file);
    std::fread(&channels, 1, 1, file);
    std::fread(&colorspace, 1, 1, file);

    image.width  = (int)width [3] | (int)width [2] << 8 | (int)width [1] << 16 | (int)width [0] << 24;
    image.height = (int)height[3] | (int)height[2] << 8 | (int)height[1] << 16 | (int)height[0] << 24;

    if (verbose) std::puts("Reading and unpacking data . . .");

    // Allocate whole image
    image.pixels = new pixel[image.height * image.width];

    size_t nbDone = 0;      // Count of how many pixels have been treated

    uint64_t checker = 0;   // Used to check if file is ending, denoted by this sliding window being == 1

    pixel prev = pixel{0, 0, 0, 255};
    pixel array[64] = {};

    uint8_t lumaDiffG;

    uint8_t tag;

    while (checker != 1 && nbDone < abs(image.height * image.width))
    // while (nbDone < 40)
    {
        std::fread(&tag, 1, 1, file);
        if (tag == QOI_OP_RGB) {
            // Read QOI_OP_RGB
            std::fread(&prev, 1, 3, file);
            prev.a = 255;
            image.pixels[nbDone++] = prev;
            array[ QOI_HASH(prev) ] = prev;
        }
        else if (tag == QOI_OP_RGBA) {
            // Read QOI_OP_RGBA
            std::fread(&prev, 1, 4, file);
            image.pixels[nbDone++] = prev;
            array[ QOI_HASH(prev) ] = prev;
        }
        else if ((tag & 0b11000000) == QOI_OP_INDEX) {
            // Read QOI_OP_INDEX
            prev = array[tag & 0b00111111];
            image.pixels[nbDone++] = prev;
        }
        else if ((tag & 0b11000000) == QOI_OP_DIFF) {
            // Read QOI_OP_DIFF
            prev.r = (prev.r + ((tag & 0b00110000) >> 4) - 2) & 0b11111111;
            prev.g = (prev.g + ((tag & 0b00001100) >> 2) - 2) & 0b11111111;
            prev.b = (prev.b + ((tag & 0b00000011) >> 0) - 2) & 0b11111111;
            image.pixels[nbDone++] = prev;
            array[ ((3*prev.r) + (5*prev.g) + (7*prev.b) + (11*prev.a)) & 0b111111 ] = prev;
        }
        else if ((tag & 0b11000000) == QOI_OP_LUMA) {
            // Read QOI_OP_LUMA
            lumaDiffG = (tag & 0b00111111) - 32;
            std::fread(&tag, 1, 1, file);

            prev.r = (prev.r + lumaDiffG + ((tag & 0b11110000) >> 4) - 8) & 0b11111111;
            prev.g = (prev.g + lumaDiffG) & 0b11111111;
            prev.b = (prev.b + lumaDiffG + ((tag & 0b00001111) >> 0) - 8) & 0b11111111;

            image.pixels[nbDone++] = prev;
            array[ ((3*prev.r) + (5*prev.g) + (7*prev.b) + (11*prev.a)) & 0b111111 ] = prev;
        }
        else if ((tag & 0b11000000) == QOI_OP_RUN) {
            // Read QOI_OP_RUN
            for (int i = -1; i < (tag & 0b00111111); i++)
            {
                image.pixels[nbDone++] = prev;
            }
        }
    }

    return image;
}

void qoi::write(std::string filename, image_data *data)
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

    fwrite("qoif", 1, 4, file);

    char width[4] = {
        (char)((data->width & 0xFF000000) >> 24),
        (char)((data->width & 0xFF0000) >> 16),
        (char)((data->width & 0xFF00) >> 8),
        (char)(data->width & 0xFF)
    };
    char height[4] = {
        (char)((data->height & 0xFF000000) >> 24),
        (char)((data->height & 0xFF0000) >> 16),
        (char)((data->height & 0xFF00) >> 8),
        (char)(data->height & 0xFF)
    };

    fwrite(&width, 1, 4, file);
    fwrite(&height, 1, 4, file);

    uint8_t channels = 4, colorspace = 0;

    fwrite(&channels, 1, 1, file);
    fwrite(&colorspace, 1, 1, file);

    uint8_t dr, dg, db, da, dr_dg, db_dg;

    pixel cache[64];
    memset(&cache, 0, sizeof(pixel[64]));

    pixel prev = pixel{0, 0, 0, 255};
    pixel curr = pixel{0, 0, 0, 255};

    uint8_t run = 0;

    uint8_t byte;

    for (int64_t iPix = 0; iPix < data->height * data->width; iPix++)
    {
        prev = curr;
        curr = data->pixels[iPix];
        // Do QOI_OP_RUN
        if (curr == prev) {     // Check if run continues
            run++;
            if (run == 62 || iPix == ((data->height * data->width) - 1)) { // Max run length in one chunk
                run = (run-1) | QOI_OP_RUN;
                std::fwrite(&run, 1, 1, file);
                run = 0;
            } 
            cache[ QOI_HASH(curr) ] = curr;
            continue;
        }
        if (run >= 1 && not(curr == prev)) {   // If it stops or must be dumped to fit in chunk
            run = (run-1) | QOI_OP_RUN;
            std::fwrite(&run, 1, 1, file);
            run = 0;
        }

        // Do QOI_OP_INDEX
        if (cache[ QOI_HASH(curr) ] == curr) {     // If it matches, write it
            byte = (QOI_HASH(curr) & 0b111111) | QOI_OP_INDEX;
            fwrite(&byte, 1, 1, file);
            cache[ QOI_HASH(curr) ] = curr;
            continue;
        }
        // calc diff
        dr = curr.r - prev.r;
        dg = curr.g - prev.g;
        db = curr.b - prev.b;
        da = curr.a - prev.a;
        dr_dg = dr - dg;
        db_dg = db - dg;
        // take wraparound into account
        dr = -MIN_WRAP(dr);
        dg = -MIN_WRAP(dg);
        db = -MIN_WRAP(db);
        da = -MIN_WRAP(da);
        dr_dg = -MIN_WRAP(dr_dg);
        db_dg = -MIN_WRAP(db_dg);

        if ((dr >= -2 && dr <= 1) && (dg >= -2 && dg <= 1) && (db >= -2 && db <= 1) && (da == 0)) {
            // std::puts("diff");
            byte = QOI_OP_DIFF | (((dr+2) & 0b11) << 4 ) | (((dg+2) & 0b11) << 2 ) | (((db+2) & 0b11) << 0 );
            fwrite(&byte, 1, 1, file);
        }
        else if ((dg >= -32 && dg <= 31) && (dr_dg >= -8 && dr_dg <= 7) && (db_dg >= -8 && db_dg <= 7) && (da == 0)) {
            // std::puts("luma");
            byte = QOI_OP_LUMA | ((dg+32) & 0b111111);
            fwrite(&byte, 1, 1, file);
            byte = (((dr_dg+8) & 0b1111) << 4) | (((db_dg+8) & 0b1111) << 0);
            fwrite(&byte, 1, 1, file);
        }
        else if (curr.a == 255) {
            byte = QOI_OP_RGB;
            fwrite(&byte, 1, 1, file);
            fwrite(&curr, 1, 3, file);
        }
        else {
            byte = QOI_OP_RGBA;
            fwrite(&byte, 1, 1, file);
            fwrite(&curr, 1, 4, file);
        }
        cache[ QOI_HASH(curr) ] = curr;

    }

    uint8_t end[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    fwrite(&end, 1, 8, file);

    fclose(file);

}
