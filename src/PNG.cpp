#include "PNG.hpp"
#include <vector>

#define Error(msg_format...)                    \
(                                               \
    fprintf(stderr, "[ERROR] " msg_format),     \
    exit(EXIT_FAILURE)                          \
)
#define Warning(msg_format...)                  \
(                                               \
    fprintf(stderr, "[WARNING] " msg_format)    \
)

void invertEndianness(unsigned* value)
{
    const char* bytes = (char*)value;
    *value = 
    bytes[0] << 24 |
    bytes[1] << 16 |
    bytes[2] << 8  |
    bytes[3];
}

typedef struct _png_chunk {
    long position;
    unsigned length;
} png_chunk;
typedef struct _png_chunks {
    std::vector<png_chunk> IDAT;
    png_chunk IHDR;
    png_chunk PLTE;
    // IEND is not saved but still detected to end search
} png_chunks;

png_chunks parse_png_chunks(FILE* file)
{
    long whereBefore = std::ftell(file);
    png_chunks chunks{
        .IHDR{.position = -1},
        .PLTE{.position = -1},
    };

    std::fseek(file, 8, SEEK_SET);

    bool
    foundIHDR = false, 
    foundPLTE = false,
    foundIEND = false;

    do
    {
        png_chunk chunk = {
            .position = std::ftell(file)
        };
        std::fread(&chunk.length, 4, 1, file);
        invertEndianness(&chunk.length);


        char type[4];
        std::fread(type, 1, 4, file);

        std::printf("Chunk at %ld, of length %u and type %.4s.\n", chunk.position, chunk.length, type);

        if (std::strncmp(type, "IHDR", 4) == 0) {
            if (foundIHDR) {
                Warning("Can't have two IHDR chunks, skipping second one.\n");
            } else {
                if (foundPLTE || chunks.IDAT.size() != 0) {
                    Warning("IHDR must come first but didn't, but we'll see how it goes eh...\n");
                }
                foundIHDR = true;
                chunks.IHDR = chunk;
            }
        } else if (std::strncmp(type, "PLTE", 4) == 0) {
            if (foundPLTE) {
                Warning("Can't have two PLTE chunks, skipping second one.\n");
            } else {
                foundPLTE = true;
                chunks.PLTE = chunk;
            }
        } else if (std::strncmp(type, "IDAT", 4) == 0) {
            chunks.IDAT.push_back(chunk);
        } else if (std::strncmp(type, "IEND", 4) == 0) {
            foundIEND = true;
        } else {
            Warning("Skipping chunk %.4s.\n", type);
        }
        std::fseek(file, chunk.length + 4, SEEK_CUR);
    }
    while (!foundIEND);


    std::fseek(file, whereBefore, SEEK_SET);
    return chunks;
}


class bitstream {
public:
    uint8_t* data;
    unsigned where = 0;
    unsigned bit = 0;
    bitstream(uint8_t* source) : data(source) {}
    size_t get_bits(unsigned count)
    {
        size_t out;
        if (bit + count <= 8) {
            out = ((data[where] << bit) & 0xFF) >> (8 - bit - count);
        } else if (bit + count <= 16) {
            out = ((*(uint16_t *)(data + where)) << bit & 0xFFFF) >> (16 - bit - count);
        } else if (bit + count <= 32) {
            out = ((*(uint32_t *)(data + where)) << bit & 0xFFFFFFFF) >> (32 - bit - count);
        } else if (bit + count <= 64) {
            out = ((*(uint64_t *)(data + where)) << bit & 0xFFFFFFFFFFFFFFFF) >> (64 - bit - count);
        } else {
            Error("nope, line %d", __LINE__);
        }
        where += (bit + count) / 8;
        bit = (bit + count) % 8;
    }
    
};


std::vector<uint8_t> inflate(uint8_t* data, size_t len)
{
    std::vector<uint8_t> out;

    if (data[0] & 0x0F != 8) Error("Zlib compression method must be 8 for png, got %hhu.\n", data[0] & 0x0F);
    if (data[0] & 0xF0 > 0x70) Error("PNG LZ77 zlib window size can't be greater than 32Kib.\n");
    int windowSize = 1 << ((data[0] >> 8) + 8);
    if (((data[0] << 8) + data[1]) % 31) Error("[inflate] " "Data is bad fsr (FCHECK didn't pass).\n");

    bitstream bs(data);

    bool finalBlock = false;


    do
    {
        size_t bHeader = bs.get_bits(3);
        finalBlock = bHeader & 0b100;
        int encoding = bHeader & 0b011;
        switch (bHeader & 0b011)
        {
        case 0b00:  // stored raw
            
            break;

        case 0b01:  // static huffman
            /* code */
            break;

        case 0b10:  // dynamic huffman
            /* code */
            break;
        
        default:
            Error("Incorrect encoding method for deflate bitstream block.\n");
            break;
        }
    }
    while (!finalBlock);
    

    



    return out;
}

image_data png::read(std::string filename)
{
    FILE* file = fopen(filename.c_str(), "rb");

    {
        char magic[8];
        fread(magic, 1, 8, file);
        if (strncmp(magic, "\x89PNG\r\n\x1A\n", 8)) {
            std::fprintf(stderr, "Error: file %s is not a PNG file (magic number does not match)\n", filename.c_str());
            exit(EXIT_FAILURE);
        }
    }


    png_chunks chunks = parse_png_chunks(file);

    if (chunks.IHDR.position == -1) Error("Image header missing.\n");
    fseek(file, chunks.IHDR.position + 8, SEEK_SET);
    
    struct {
        unsigned width, height;
        unsigned char bitDepth, colorType, compression, filer, interlace;
    } header;
    fread(&header, sizeof(header), 1, file);
    invertEndianness(&header.width); invertEndianness(&header.height);
    if (header.width == 0 || header.height == 0) Error("Image dimensions can't be 0.\n");
    if (header.interlace != 0) Error("Interlace not currently supported.\n");

    image_data image(header.width, header.height);

    size_t dataSize = 0;
    for (size_t i = 0; i < chunks.IDAT.size(); i++)
        dataSize += chunks.IDAT.at(i).length;
    unsigned char* data = new unsigned char[dataSize];
    for (size_t i = 0, where = 0; i < chunks.IDAT.size(); i++)
    {
        fseek(file, chunks.IDAT.at(i).position + 8, SEEK_SET);
        fread(data + where, 1, chunks.IDAT.at(i).length, file);
        where += chunks.IDAT.at(i).length;
    }
    
    std::vector<uint8_t> inflatedData = inflate(data, dataSize);





    return image;
}

void png::write(std::string filename, image_data* data)
{
    if (true) std::puts("Not available");
    std::exit(4);
}




