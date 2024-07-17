#pragma once

#include <filesystem>

#include "Settings.hpp"

typedef struct Cia_File {
    std::filesystem::path path;
    bool result;
    std::string message;
} Cia_File;

Cia_File fix_cia(const std::filesystem::path &path, const Settings &set);
