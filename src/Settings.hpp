#pragma once
#include <filesystem>

typedef struct Settings {
    std::filesystem::path bin;
    std::filesystem::path cwd;
    bool replace;
    bool verbose;
    bool quiet;
} Settings;