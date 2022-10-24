#include "controller.h"
#include <iostream>

Controller::~Controller() {
    for (Filter *filter : filters_) {
        delete filter;
    }
}

Controller *Controller::Parse(int argc, char **argv) {
    if (argc < 3) {
        MyException("not enough parameters entered");
    }
    char *input_filename = argv[2];
    char *output_filename = argv[3];
    std::vector<Filter *> filters;

    int i = 4;
    while (i < argc) {
        if (argv[i][0] != '-') {
            MyException("wrong filter name, expected '-' at the beginning");
        }
        int j = i + 1;
        while (j < argc && argv[j][0] != '-') {
            j++;
        }
        if (strcmp("crop", argv[i] + 1) == 0) {
            if (j - i != 3) {
                MyException("wrong parameters for crop filter");
            }

            std::int32_t height = static_cast<int32_t>(std::strtol(argv[i + 1], nullptr, 10));
            std::int32_t width = static_cast<int32_t>(std::strtol(argv[i + 2], nullptr, 10));
            if (height == 0L || width == 0L) {
                MyException("wrong parameters for crop filter");
            }
            filters.push_back(new Crop(width, height));
        } else if (strcmp("gs", argv[i] + 1) == 0) {
            if (j - i != 1) {
                MyException("wrong parameters for gray scale filter");
            }
            filters.push_back(new Grayscale());
        } else if (strcmp("neg", argv[i] + 1) == 0) {
            if (j - i != 1) {
                MyException("wrong parameters for negative filter");
            }
            filters.push_back(new Negative());
        } else if (strcmp("sharp", argv[i] + 1) == 0) {
            if (j - i != 1) {
                MyException("wrong parameters for sharpening filter");
            }
            filters.push_back(new Sharpening());
        } else if (strcmp("edge", argv[i] + 1) == 0) {
            if (j - i != 2) {
                MyException("wrong parameters for edge detection filter");
            }
            std::int32_t threshold = static_cast<int32_t>(std::strtol(argv[i + 1], nullptr, 10));
            if (threshold == 0L) {
                MyException("threshold is too large");
            }
            filters.push_back(new EdgeDetection(threshold));
        } else if (strcmp("blur", argv[i] + 1) == 0) {
            if (j - i != 2) {
                MyException("wrong parameters for gaussian blur filter");
            }
            double sigma = std::strtod(argv[i + 1], nullptr);
            if (sigma == 0.0) {
                MyException("wrong value for sigma");
            }
            filters.push_back(new GaussianBlur(sigma));
        } else if (strcmp("voronoi", argv[i] + 1) == 0) {
            if (j - i != 2) {
                MyException("wrong parameters for voronoi filter");
            }
            std::uint32_t cluster_count = static_cast<std::uint32_t>(std::strtol(argv[i + 1], nullptr, 10));
            if (cluster_count == 0L) {
                MyException("wrong parameter for voronoi filter");
            }
            filters.push_back(new VoronoiBlur(cluster_count));
        }
        i = j;
    }
    return new Controller(input_filename, output_filename, filters);
}

Bitmap *Controller::ReadFile() const {
    Bitmap *bitmap = nullptr;
    std::ifstream instream;

    try {
        instream.open(input_filename_, std::ios::in | std::ios::binary);
        if (!instream) {
            throw std::runtime_error("Could not open the input file");
        }
        bitmap = Bitmap::Read(instream);
        if (bitmap == nullptr) {
            throw std::runtime_error("Could not read the input file");
        }
        instream.close();

    } catch (const std::exception &e) {
        if (bitmap != nullptr) {
            delete bitmap;
        }
        if (instream.is_open()) {
            instream.close();
        }
        throw e;
    }
    return bitmap;
}

Bitmap *Controller::ApplyFilters(Bitmap *bitmap) const {

    if (filters_.empty()) {
        return bitmap;
    } else if (filters_.size() == 1) {
        return filters_[0]->Apply(bitmap);
    } else {
        Bitmap *bitmap0;
        bitmap = filters_[0]->Apply(bitmap);
        for (size_t i = 1; i < filters_.size(); ++i) {
            bitmap0 = filters_[i]->Apply(bitmap);
            delete bitmap;
            bitmap = bitmap0;
        }
        return bitmap;
    }
}

void Controller::WriteFile(Bitmap *bitmap) const {
    std::ofstream outstream;
    try {
        outstream.open(output_filename_, std::ios::out | std::ios::binary);
        if (!outstream) {
            throw std::runtime_error("Could not open the output file");
        }
        bitmap->Write(outstream);
        outstream.close();
    } catch (const std::exception &e) {
        if (outstream.is_open()) {
            outstream.close();
        }
        throw e;
    }
}