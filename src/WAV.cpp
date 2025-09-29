#include "WAV.h"
#include <variant>


audio_data wav::read(std::string filename)
{
    FILE* file = std::fopen(filename.c_str(), "rb");

    audio_data audio;

    char signature[5] = {};
    std::fread(&signature, 1, 4, file);
    if (not equals(signature, "RIFF")) { // Check RIFF signature
        if (verbose) std::printf("Invalid first file signature ( %s ) vs expected for RIFF files ( RIFF )\n", signature);
        std::fclose(file);
        std::exit(1);
    }
    std::uint32_t dataSize;
    std::fread(&dataSize, 4, 1, file);
    dataSize -= 4;

    std::fread(&signature, 1, 4, file);
    if (not equals(signature, "WAVE")) { // Check WAVE signature
        if (verbose) std::printf("Invalid second file signature ( %s ) vs expected for WAVE files ( WAVE )\n", signature);
        std::fclose(file);
        std::exit(1);
    }

    char subChunk1Id[5] = {};
    std::fread(&subChunk1Id, 1, 4, file);
    if (not equals(subChunk1Id, "fmt ")) {
        if (verbose) std::puts("File exited canonical format (SubChunk1 ID != \"fmt \"); not supported.");
        std::fclose(file);
        std::exit(1);
    }

    std::uint32_t subChunk1Size, byteRate;
    std::uint16_t audioFormat, blockAlign, bitsPerSample;

    std::fread(&subChunk1Size, 4, 1, file);
    if (subChunk1Size != 16) {
        if (verbose) std::puts("File exited canonical format (SubChunk1 Size != 16); not supported.");
        std::fclose(file);
        std::exit(1);
    }
    std::fread(&audioFormat, 2, 1, file);
    std::printf("Audio format: %hu", audioFormat);
    std::fread(&audio.numChannels, 2, 1, file);
    std::fread(&audio.sampleRate, 4, 1, file);
    std::fread(&byteRate, 4, 1, file);
    std::fread(&blockAlign, 2, 1, file);
    std::fread(&bitsPerSample, 2, 1, file);
    
    char subChunk2ID[5];
    std::fread(&subChunk2ID, 1, 4, file);
    if (not equals(subChunk2ID, "data")) {
        if (verbose) std::puts("File exited canonical format (SubChunk2 ID != \"data\"); not supported.");
        std::fclose(file);
        std::exit(1);
    }

    std::uint32_t subChunk2Size;
    std::fread(&subChunk2Size, 4, 1, file);

// ▄▄▄█████▓ ██░ ██ ▓█████      █████▒█    ██  ███▄    █      ██████ ▄▄▄█████▓ ▒█████   ██▓███    ██████     ██░ ██ ▓█████  ██▀███  ▓█████ 
// ▓  ██▒ ▓▒▓██░ ██▒▓█   ▀    ▓██   ▒ ██  ▓██▒ ██ ▀█   █    ▒██    ▒ ▓  ██▒ ▓▒▒██▒  ██▒▓██░  ██▒▒██    ▒    ▓██░ ██▒▓█   ▀ ▓██ ▒ ██▒▓█   ▀ 
// ▒ ▓██░ ▒░▒██▀▀██░▒███      ▒████ ░▓██  ▒██░▓██  ▀█ ██▒   ░ ▓██▄   ▒ ▓██░ ▒░▒██░  ██▒▓██░ ██▓▒░ ▓██▄      ▒██▀▀██░▒███   ▓██ ░▄█ ▒▒███   
// ░ ▓██▓ ░ ░▓█ ░██ ▒▓█  ▄    ░▓█▒  ░▓▓█  ░██░▓██▒  ▐▌██▒     ▒   ██▒░ ▓██▓ ░ ▒██   ██░▒██▄█▓▒ ▒  ▒   ██▒   ░▓█ ░██ ▒▓█  ▄ ▒██▀▀█▄  ▒▓█  ▄ 
//   ▒██▒ ░ ░▓█▒░██▓░▒████▒   ░▒█░   ▒▒█████▓ ▒██░   ▓██░   ▒██████▒▒  ▒██▒ ░ ░ ████▓▒░▒██▒ ░  ░▒██████▒▒   ░▓█▒░██▓░▒████▒░██▓ ▒██▒░▒████▒
//   ▒ ░░    ▒ ░░▒░▒░░ ▒░ ░    ▒ ░   ░▒▓▒ ▒ ▒ ░ ▒░   ▒ ▒    ▒ ▒▓▒ ▒ ░  ▒ ░░   ░ ▒░▒░▒░ ▒▓▒░ ░  ░▒ ▒▓▒ ▒ ░    ▒ ░░▒░▒░░ ▒░ ░░ ▒▓ ░▒▓░░░ ▒░ ░
//     ░     ▒ ░▒░ ░ ░ ░  ░    ░     ░░▒░ ░ ░ ░ ░░   ░ ▒░   ░ ░▒  ░ ░    ░      ░ ▒ ▒░ ░▒ ░     ░ ░▒  ░ ░    ▒ ░▒░ ░ ░ ░  ░  ░▒ ░ ▒░ ░ ░  ░
//   ░       ░  ░░ ░   ░       ░ ░    ░░░ ░ ░    ░   ░ ░    ░  ░  ░    ░      ░ ░ ░ ▒  ░░       ░  ░  ░      ░  ░░ ░   ░     ░░   ░    ░   
//           ░  ░  ░   ░  ░             ░              ░          ░               ░ ░                 ░      ░  ░  ░   ░  ░   ░        ░  ░
// 
// 
//  ██╗ █████╗ ██╗  ██╗ █████╗      █████╗  ██████╗████████╗██╗   ██╗ █████╗ ██╗         ██████╗  █████╗ ████████╗ █████╗ ██╗ 
// ██╔╝██╔══██╗██║ ██╔╝██╔══██╗    ██╔══██╗██╔════╝╚══██╔══╝██║   ██║██╔══██╗██║         ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗╚██╗
// ██║ ███████║█████╔╝ ███████║    ███████║██║        ██║   ██║   ██║███████║██║         ██║  ██║███████║   ██║   ███████║ ██║
// ██║ ██╔══██║██╔═██╗ ██╔══██║    ██╔══██║██║        ██║   ██║   ██║██╔══██║██║         ██║  ██║██╔══██║   ██║   ██╔══██║ ██║
// ╚██╗██║  ██║██║  ██╗██║  ██║    ██║  ██║╚██████╗   ██║   ╚██████╔╝██║  ██║███████╗    ██████╔╝██║  ██║   ██║   ██║  ██║██╔╝
//  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝    ╚═╝  ╚═╝ ╚═════╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝    ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝ 

    audio.bytesPerSample = bitsPerSample / 8;

    audio.numSamples = subChunk2Size / (audio.bytesPerSample * audio.numChannels);
    

    audio.samples = (void**)std::malloc(sizeof(void*) * audio.numChannels);
    for (uint16_t i = 0; i < audio.numChannels; i++)
    {
        audio.samples[i] = (void*)std::malloc(audio.bytesPerSample * audio.numSamples);
    }
    
    for (uint32_t n_sample = 0; n_sample < audio.numSamples; n_sample++)
    {
        for (uint16_t channel = 0; channel < audio.numChannels; channel++)
        {
            std::fread(audio.samples[channel] + (n_sample*audio.bytesPerSample), audio.bytesPerSample, 1, file);
        }
    }

    return audio;
}

void wav::write(std::string filename, audio_data *data)
{
    FILE* file = std::fopen(filename.c_str(), "wb");

    std::fwrite("RIFF", 1, 4, file);

    std::uint32_t dataSize = data->numSamples * data->numChannels * data->bytesPerSample;
    std::uint32_t fileSize = 36 + dataSize;

    std::fwrite(&fileSize, 4, 1, file);

    std::fwrite("WAVE", 1, 4, file);
    std::fwrite("fmt ", 1, 4, file);

    std::uint32_t fmtChunkSize = 16,
            byteRate = data->sampleRate * data->numChannels * data->bytesPerSample;
    
    std::uint16_t audioFormat = 1, // 1 for PCM
            blockAlign = data->bytesPerSample * data->numChannels,
            bitsPerSample = data->bytesPerSample * 8;

    std::fwrite(&fmtChunkSize, 4, 1, file);
    std::fwrite(&audioFormat, 2, 1, file);
    std::fwrite(&data->numChannels, 2, 1, file);
    std::fwrite(&data->sampleRate, 4, 1, file);
    std::fwrite(&byteRate, 4, 1, file);
    std::fwrite(&blockAlign, 2, 1, file);
    std::fwrite(&bitsPerSample, 2, 1, file);

    std::fwrite("data", 1, 4, file);

    std::fwrite(&dataSize, 4, 1, file);
    for (uint32_t n_sample = 0; n_sample < data->numSamples; n_sample++)
    {
        for (uint16_t channel = 0; channel < data->numChannels; channel++)
        {
            std::fwrite(data->samples[channel] + (n_sample*data->bytesPerSample), data->bytesPerSample, 1, file);
        }
    }
    
}

