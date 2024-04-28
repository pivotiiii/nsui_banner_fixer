#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>

typedef struct Settings {
    std::filesystem::path bin;
    std::filesystem::path cwd;
    std::filesystem::path dstool;
    std::filesystem::path ctrtool;
    std::filesystem::path makerom;
    bool replace;
    bool verbose;
    bool quiet;
} Settings;

#endif
