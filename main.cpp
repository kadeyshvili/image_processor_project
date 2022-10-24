#include <string>
#include <iostream>
#include <optional>
#include "controller.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout
                << "{имя программы} {путь к входному файлу} {путь к выходному файлу} [-{имя фильтра 1} [параметр фильтра "
                   "1] [параметр фильтра 2] ...] [-{имя фильтра 2} [параметр фильтра 1] [параметр фильтра 2] ...] ..."
                << std::endl;
        return 0;
    }
    Bitmap* bitmap = nullptr;
    Bitmap* bitmap1 = nullptr;
    Controller* controller;
    try {
        controller = Controller::Parse(argc, argv);
        bitmap = controller->ReadFile();
        bitmap1 = controller->ApplyFilters(bitmap);
        controller->WriteFile(bitmap1);
        delete controller;
        delete bitmap;
        delete bitmap1;
    } catch (const std::exception& e) {
        if (controller != nullptr) {
            delete controller;
        }
        if (bitmap != nullptr) {
            delete bitmap;
        }
        if (bitmap1 != nullptr) {
            delete bitmap1;
        }
        std::cerr << e.what() << std::endl;
    }
    return 0;
}