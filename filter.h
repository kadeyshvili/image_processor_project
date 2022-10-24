#pragma once

#include "bitmap.h"
#include <algorithm>
#include <vector>

class Filter {
public:
    virtual ~Filter(){};

    virtual Bitmap *Apply(const Bitmap *bitmap) = 0;
};

class Crop : public Filter {
public:
    Crop(int32_t width, int32_t height) : width_(width), height_(height) {
    }

    Bitmap *Apply(const Bitmap *bitmap) override;
    void UpdateSize(BitmapFileHeader &file_header, BitmapInfoHeader &info_header);

private:
    int32_t width_;
    int32_t height_;
};


class Grayscale : public Filter {
public:
    Bitmap *Apply(const Bitmap *bitmap) override;
};

class Negative : public Filter {
public:
    Bitmap *Apply(const Bitmap *bitmap) override;
};

class Sharpening : public Filter {
public:
    Bitmap *Apply(const Bitmap *bitmap) override;
};

class EdgeDetection : public Filter {
public:
    explicit EdgeDetection(int32_t threshold) : threshold_(threshold) {
    }

    Bitmap *Apply(const Bitmap *bitmap) override;

private:
    int32_t threshold_;
};

class GaussianBlur : public Filter {
public:
    explicit GaussianBlur(double sigma) : sigma_(sigma) {
    }
    Bitmap *Apply(const Bitmap *bitmap) override;

private:
    double sigma_;
};

class VoronoiBlur : public Filter {
public:
    VoronoiBlur(std::uint32_t cluster_count)
            : cluster_count_(cluster_count){
    }
    Bitmap *Apply(const Bitmap *bitmap);

private:
    std::uint32_t cluster_count_;
};