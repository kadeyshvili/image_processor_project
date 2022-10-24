#pragma once

#include <utility>
#include <vector>
#include <iostream>
#include <optional>
#include "filter.h"

class MyException : public std::exception {
public:
    MyException(const char *message) : message_(message) {
        std::cerr << message_;
    }

protected:
    std::string message_;
};

class Controller {
public:
    Controller(){}
    Controller(char *input_filename, char *output_filename, std::vector<Filter *> filters)
            : input_filename_(input_filename), output_filename_(output_filename), filters_(std::move(filters)) {
    }

    ~Controller();
    static Controller* Parse(int argc, char **argv);
    Bitmap* ReadFile() const;
    Bitmap* ApplyFilters(Bitmap* bitmap) const;
    void WriteFile(Bitmap* bitmap) const;

private:
    char *input_filename_;
    char *output_filename_;
    std::vector<Filter *> filters_;
};
