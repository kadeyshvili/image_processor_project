#include "bitmap.h"

Bitmap *Bitmap::Read(std::ifstream &instream) {
    instream.seekg(0, std::ios::end);
    std::size_t length = instream.tellg();
    instream.seekg(0, std::ios::beg);

    if (length < sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader)) {
        return nullptr;
    }

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    instream.read(reinterpret_cast<char *>(&file_header), sizeof(BitmapFileHeader));
    instream.read(reinterpret_cast<char *>(&info_header), sizeof(BitmapInfoHeader));

    size_t height = info_header.biHeight;
    size_t width = info_header.biWidth;

    if (length < file_header.bfSize) {
        return nullptr;
    }

    std::vector<std::vector<Color>> data(height, std::vector<Color>(width));
    auto padding_size = static_cast<std::streamsize>(width * 3 % 4);

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            instream.read(reinterpret_cast<char *>(&(data[i][j])), sizeof(Color));
        }
        if (padding_size) {
            int32_t padding;
            instream.read(reinterpret_cast<char *>(&padding), 4 - padding_size);
        }
    }

    return new Bitmap(file_header, info_header, data);
}

void Bitmap::Write(std::ofstream &outstream) const {
    outstream.write((char *)(&file_header_), sizeof(BitmapFileHeader));
    outstream.write((char *)(&info_header_), sizeof(BitmapInfoHeader));
    if ((info_header_.biWidth * sizeof(Color)) % 4 != 0) {
        std::int32_t x = 0;
        for (std::size_t i = 0; i < static_cast<size_t>(info_header_.biHeight); i++) {
            for (std::size_t j = 0; j < static_cast<size_t>(info_header_.biWidth); j++) {
                outstream.write((char *)(&(data_[i][j])), sizeof(Color));
            }
            outstream.write(reinterpret_cast<char *>(&(x)), 4 - (info_header_.biWidth * sizeof(Color)) % 4);
        }
        for (std::size_t i = 0; i < static_cast<size_t>(info_header_.biHeight); i++) {
            for (std::size_t j = 0; j < static_cast<size_t>(info_header_.biWidth); j++) {
                outstream.write((char *)(&(data_[i][j])), sizeof(Color));
            }
        }
    } else {
        for (std::size_t i = 0; i < static_cast<size_t>(info_header_.biHeight); i++) {
            for (std::size_t j = 0; j < static_cast<size_t>(info_header_.biWidth); j++) {
                outstream.write((char *)(&(data_[i][j])), sizeof(Color));
            }
        }
    }
}

std::int32_t Bitmap::GetHeight() const {
    return info_header_.biHeight;
}

std::int32_t Bitmap::GetWidth() const {
    return info_header_.biWidth;
}

BitmapFileHeader Bitmap::GetFileHeader() const {
    return file_header_;
}

BitmapInfoHeader Bitmap::GetInfoHeader() const {
    return info_header_;
}

Color Bitmap::GetData(size_t i, size_t j) const {
    return data_[i][j];
}