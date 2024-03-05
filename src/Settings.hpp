#ifndef SETTINGS_H
#define SETTINGS_H
#include <filesystem>

typedef struct Settings {
    std::filesystem::path bin;
    bool replace;
    bool verbose;
    bool quiet;
} Settings;

#endif