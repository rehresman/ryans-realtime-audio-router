#pragma once
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

class WavWriter16Mono {
public:

    static bool write(const std::string& path,
                      const std::vector<float>& samples,
                      uint32_t sampleRate)
                      
    {
        
        // Convert float [-1,1] to int16
        std::vector<int16_t> pcm;
        pcm.reserve(samples.size());
        for (float x : samples) {
            x = std::clamp(x, -1.0f, 1.0f);
            int32_t v = static_cast<int32_t>(x * 32767.0f);
            pcm.push_back(static_cast<int16_t>(v));
        }

        std::ofstream out(path, std::ios::binary);
        if (!out) return false;

        const uint16_t numChannels = 1;
        const uint16_t bitsPerSample = 16;
        const uint16_t audioFormat = 1; // PCM
        const uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
        const uint16_t blockAlign = numChannels * (bitsPerSample / 8);

        const uint32_t dataSize = static_cast<uint32_t>(pcm.size() * sizeof(int16_t));
        const uint32_t riffSize = 36u + dataSize;

        // RIFF header
        out.write("RIFF", 4);
        write_u32(out, riffSize);
        out.write("WAVE", 4);

        // fmt chunk
        out.write("fmt ", 4);
        write_u32(out, 16u);                 // PCM fmt chunk size
        write_u16(out, audioFormat);
        write_u16(out, numChannels);
        write_u32(out, sampleRate);
        write_u32(out, byteRate);
        write_u16(out, blockAlign);
        write_u16(out, bitsPerSample);

        // data chunk
        out.write("data", 4);
        write_u32(out, dataSize);
        out.write(reinterpret_cast<const char*>(pcm.data()), dataSize);

        return true;
    }

private:
    static void write_u16(std::ofstream& out, uint16_t v) {
        char b[2] = { static_cast<char>(v & 0xFF), static_cast<char>((v >> 8) & 0xFF) };
        out.write(b, 2);
    }
    static void write_u32(std::ofstream& out, uint32_t v) {
        char b[4] = {
            static_cast<char>(v & 0xFF),
            static_cast<char>((v >> 8) & 0xFF),
            static_cast<char>((v >> 16) & 0xFF),
            static_cast<char>((v >> 24) & 0xFF)
        };
        out.write(b, 4);
    }
};
