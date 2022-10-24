#pragma once

#include "stdint.h"
#include <fstream>
#include <vector>

struct BitmapFileHeader {
    std::uint16_t bfType;
    std::uint32_t bfSize;
    std::uint16_t bfReserved1;
    std::uint16_t bfReserved2;
    std::uint32_t bfOffBits;
} __attribute__((__packed__));

struct BitmapInfoHeader {
    std::uint32_t biSize;
    std::int32_t biWidth;
    std::int32_t biHeight;
    std::uint16_t biPlanes;
    std::uint16_t biBitCount;
    std::uint32_t biCompression;
    std::uint32_t biSizeImage;
    std::int32_t biXPelsPerMeter;
    std::int32_t biYPelsPerMeter;
    std::uint32_t biClrUsed;
    std::uint32_t biClrImportant;
} __attribute__((__packed__));

struct Color {
    std::uint8_t blue;
    std::uint8_t green;
    std::uint8_t red;
} __attribute__((__packed__));

class Bitmap {
public:
    Bitmap(const BitmapFileHeader &file_header, const BitmapInfoHeader &info_header,
           std::vector<std::vector<Color>> data)
            : file_header_(file_header), info_header_(info_header), data_(data) {
    }

    static Bitmap *Read(std::ifstream &instream);

    void Write(std::ofstream &outstream) const;

    std::int32_t GetHeight() const;

    std::int32_t GetWidth() const;

    BitmapFileHeader GetFileHeader() const;

    BitmapInfoHeader GetInfoHeader() const;

    Color GetData(std::size_t i, std::size_t j) const;

private:
    BitmapFileHeader file_header_;
    BitmapInfoHeader info_header_;
    std::vector<std::vector<Color>> data_;
};
