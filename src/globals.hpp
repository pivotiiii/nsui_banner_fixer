#ifndef GLOBALS_H
#define GLOBALS_H

#include <filesystem>

inline const std::vector<int> locale_offsets = {0x14BC, 0x14CB};

inline const std::vector<std::string> locale_codes = {
    "EUR_EN",
    "EUR_FR",
    "EUR_GE",
    "EUR_IT",
    "EUR_SP",
    "EUR_DU",
    "EUR_PO",
    "EUR_RU",
    "JPN_JP",
    "USA_EN",
    "USA_FR",
    "USA_SP",
    "USA_PO"};

inline std::filesystem::path dstool = std::filesystem::path("tools") / "3dstool.exe";
inline std::filesystem::path ctrtoolX = std::filesystem::path("tools") / "ctrtool.exe";
inline std::filesystem::path makerom = std::filesystem::path("tools") / "makerom.exe";

#endif
