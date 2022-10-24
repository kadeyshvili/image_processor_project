#include "filter.h"
#include <cmath>
#include <random>
#include <limits>

void Crop::UpdateSize(BitmapFileHeader &file_header, BitmapInfoHeader &info_header) {
    file_header.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + width_ * height_ * sizeof(Color);
    info_header.biHeight = height_;
    info_header.biWidth = width_;
    info_header.biSizeImage = height_ * (((width_ * sizeof(Color) + 3) / 4) * 4);
    file_header.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + info_header.biSizeImage;
}

Bitmap *Crop::Apply(const Bitmap *bitmap) {
    int32_t height = bitmap->GetHeight();
    int32_t width = bitmap->GetWidth();
    BitmapFileHeader file_header = bitmap->GetFileHeader();
    BitmapInfoHeader info_header = bitmap->GetInfoHeader();
    if (height_ > height || width_ > width || height_ < 0 || width_ < 0) {
        height_ = height;
        width_ = width;
    }
    UpdateSize(file_header, info_header);
    std::vector<std::vector<Color>> data(height_, std::vector<Color>(width_));
    for (size_t i = 0; i < static_cast<size_t>(height_); ++i) {
        for (size_t j = 0; j < static_cast<size_t>(width_); ++j) {
            data[i][j] = bitmap->GetData(height - height_ + i, j);
        }
    }
    return new Bitmap(file_header, info_header, data);
}

Bitmap *Grayscale::Apply(const Bitmap *bitmap) {
    BitmapFileHeader file_header = bitmap->GetFileHeader();
    BitmapInfoHeader info_header = bitmap->GetInfoHeader();
    size_t height = static_cast<size_t>(bitmap->GetHeight());
    size_t width = static_cast<size_t>(bitmap->GetWidth());
    std::vector<std::vector<Color>> data(height, std::vector<Color>(width));
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            Color pixel = bitmap->GetData(i, j);
            std::uint32_t grey = 0.299 * pixel.red + 0.587 * pixel.green + 0.114 * pixel.blue;
            data[i][j].red = grey;
            data[i][j].blue = grey;
            data[i][j].green = grey;
        }
    }
    return new Bitmap(file_header, info_header, data);
}

Bitmap *Negative::Apply(const Bitmap *bitmap) {
    BitmapFileHeader file_header = bitmap->GetFileHeader();
    BitmapInfoHeader info_header = bitmap->GetInfoHeader();
    size_t height = static_cast<size_t>(bitmap->GetHeight());
    size_t width = static_cast<size_t>(bitmap->GetWidth());
    std::vector<std::vector<Color>> data(height, std::vector<Color>(width));
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            Color pixel = bitmap->GetData(i, j);
            data[i][j].red = 255 - pixel.red;
            data[i][j].blue = 255 - pixel.blue;
            data[i][j].green = 255 - pixel.green;
        }
    }
    return new Bitmap(file_header, info_header, data);
}

Bitmap *Sharpening::Apply(const Bitmap *bitmap) {
    BitmapFileHeader file_header = bitmap->GetFileHeader();
    BitmapInfoHeader info_header = bitmap->GetInfoHeader();
    int32_t height = bitmap->GetHeight();
    int32_t width = bitmap->GetWidth();
    std::vector<std::vector<Color>> data(height, std::vector<Color>(width));
    std::vector<std::vector<Color>> buffer(3, std::vector<Color>(3));
    for (int32_t i = 0; i < height; ++i) {
        for (int32_t j = 0; j < width; ++j) {
            for (int32_t i0 = 0; i0 < 3; ++i0) {
                for (int32_t j0 = 0; j0 < 3; ++j0) {
                    buffer[i0][j0] = bitmap->GetData(std::max(0, std::min(height - 1, i + i0 - 1)),
                                                     std::max(0, std::min(width - 1, j + j0 - 1)));
                }
            }
            int32_t red =
                    5 * buffer[1][1].red - buffer[0][1].red - buffer[1][0].red - buffer[1][2].red - buffer[2][1].red;
            data[i][j].red = std::min(255, std::max(0, red));
            int32_t green = 5 * buffer[1][1].green - buffer[0][1].green - buffer[1][0].green - buffer[1][2].green -
                            buffer[2][1].green;
            data[i][j].green = std::min(255, std::max(0, green));
            int32_t blue =
                    5 * buffer[1][1].blue - buffer[0][1].blue - buffer[1][0].blue - buffer[1][2].blue - buffer[2][1].blue;
            data[i][j].blue = std::min(255, std::max(0, blue));
        }
    }
    return new Bitmap(file_header, info_header, data);
}

Bitmap *EdgeDetection::Apply(const Bitmap *bitmap) {
    Grayscale operation;
    Bitmap *bitmap0 = operation.Apply(bitmap);
    BitmapFileHeader file_header = bitmap->GetFileHeader();
    BitmapInfoHeader info_header = bitmap->GetInfoHeader();
    int32_t height = bitmap->GetHeight();
    int32_t width = bitmap->GetWidth();
    std::vector<std::vector<Color>> data(height, std::vector<Color>(width));
    std::vector<std::vector<Color>> buffer(3, std::vector<Color>(3));
    for (int32_t i = 0; i < height; ++i) {
        for (int32_t j = 0; j < width; ++j) {
            for (int32_t i0 = 0; i0 < 3; ++i0) {
                for (int32_t j0 = 0; j0 < 3; ++j0) {
                    buffer[i0][j0] = bitmap0->GetData(std::max(0, std::min(height - 1, i + i0 - 1)),
                                                      std::max(0, std::min(width - 1, j + j0 - 1)));
                }
            }
            int32_t grey =
                    4 * buffer[1][1].red - buffer[0][1].red - buffer[1][0].red - buffer[1][2].red - buffer[2][1].red;
            if (grey > threshold_) {
                data[i][j] = {255, 255, 255};
            } else {
                data[i][j] = {0, 0, 0};
            }
        }
    }
    delete bitmap0;
    return new Bitmap(file_header, info_header, data);
}

Bitmap *GaussianBlur::Apply(const Bitmap *bitmap) {
    BitmapFileHeader file_header = bitmap->GetFileHeader();
    BitmapInfoHeader info_header = bitmap->GetInfoHeader();

    std::int32_t height = bitmap->GetHeight();
    std::int32_t width = bitmap->GetWidth();

    std::vector<std::vector<Color>> data(height, std::vector<Color>(width));

    std::int32_t sigma3 = static_cast<int32_t>(sigma_ * 3.0);
    std::int32_t sigma_full = sigma3 * 2 + 1;

    double s = 0.0;
    for (std::int32_t i = 0; i < sigma_full; ++i) {
        s += std::exp(-((i - sigma3) * (i - sigma3)) / (2 * sigma_ * sigma_)) / (sigma_ * std::sqrt(2.0 * M_PI));
    }

    std::vector<double> gauss_buf(sigma_full);
    for (std::int32_t i = 0; i < sigma_full; ++i) {
        gauss_buf[i] =
                std::exp(-((i - sigma3) * (i - sigma3)) / (2 * sigma_ * sigma_)) / (sigma_ * std::sqrt(2.0 * M_PI)) / s;
    }

    std::vector<Color> buf(std::max(height, width));

    for (std::int32_t j = 0; j < width; ++j) {
        for (std::int32_t i = 0; i < height; ++i) {
            buf[i] = bitmap->GetData(i, j);
        }
        for (std::int32_t i = 0; i < height; ++i) {
            double red = 0.0;
            double green = 0.0;
            double blue = 0.0;
            for (std::int32_t i0 = 0; i0 < sigma_full; ++i0) {
                std::int32_t i1 = std::max(0, std::min(height - 1, i + i0 - sigma3));
                red += buf[i1].red * gauss_buf[i0] / 255.0;
                green += buf[i1].green * gauss_buf[i0] / 255.0;
                blue += buf[i1].blue * gauss_buf[i0] / 255.0;
            }
            data[i][j].blue = blue > 1 ? 255 : static_cast<uint8_t>(blue * 255);
            data[i][j].green = green > 1 ? 255 : static_cast<uint8_t>(green * 255);
            data[i][j].red = red > 1 ? 255 : static_cast<uint8_t>(red * 255);
        }
    }
    for (std::int32_t i = 0; i < height; ++i) {
        for (std::int32_t j = 0; j < width; ++j) {
            buf[j] = data[i][j];
        }
        for (std::int32_t j = 0; j < width; ++j) {
            double red = 0.0;
            double green = 0.0;
            double blue = 0.0;
            for (std::int32_t j0 = 0; j0 < sigma_full; ++j0) {
                std::int32_t j1 = std::max(0, std::min(width - 1, j + j0 - sigma3));
                red += buf[j1].red * gauss_buf[j0] / 255.0;
                green += buf[j1].green * gauss_buf[j0] / 255.0;
                blue += buf[j1].blue * gauss_buf[j0] / 255.0;
            }
            data[i][j].blue = blue > 1 ? 255 : static_cast<uint8_t>(blue * 255);
            data[i][j].green = green > 1 ? 255 : static_cast<uint8_t>(green * 255);
            data[i][j].red = red > 1 ? 255 : static_cast<uint8_t>(red * 255);
        }
    }
    return new Bitmap(file_header, info_header, data);
}

Bitmap *VoronoiBlur::Apply(const Bitmap *bitmap) {
    BitmapFileHeader file_header = bitmap->GetFileHeader();
    BitmapInfoHeader info_header = bitmap->GetInfoHeader();

    std::int32_t height = bitmap->GetHeight();
    std::int32_t width = bitmap->GetWidth();

    std::vector<std::vector<Color>> data(height, std::vector<Color>(width));

    std::random_device srand;
    std::mt19937 rng(srand());
    std::uniform_int_distribution<> height_distribution(0, height - 1);
    std::uniform_int_distribution<> width_distribution(0, width - 1);

    std::vector<std::pair<std::int32_t, std::int32_t>> points(cluster_count_);
    for (std::uint32_t i = 0; i < cluster_count_; i++) {
        points[i] = {height_distribution(rng), width_distribution(rng)};
    }

    std::int32_t current_nearest_point, a, b, distance, new_distance;
    for (std::int32_t i = 0; i < height; i++) {
        for (std::int32_t j = 0; j < width; j++) {
            current_nearest_point = 0;
            distance = std::numeric_limits<int>::max();
            for (std::int32_t new_nearest_point = 1; new_nearest_point < static_cast<int32_t>(points.size());
                 new_nearest_point++) {
                a = i - points[new_nearest_point].first;
                b = j - points[new_nearest_point].second;
                new_distance = a * a + b * b;
                if (new_distance < distance) {
                    current_nearest_point = new_nearest_point;
                    distance = new_distance;
                }
            }
            data[i][j] = bitmap->GetData(points[current_nearest_point].first, points[current_nearest_point].second);
        }
    }
    return new Bitmap(file_header, info_header, data);
}