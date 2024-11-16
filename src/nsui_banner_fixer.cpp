#include <filesystem>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include "Windows.h"
#endif

#include "Game.hpp"
#include "Settings.hpp"
#include "nsui_banner_fixer.hpp"

#ifdef GUI
#include "UI.hpp"
#else
#include "ArgsParser.hpp"
#endif

#include <pathfind.hpp>

#ifndef VERSION
#define VERSION "0.0.0"
#endif
#ifndef YEAR
#define YEAR "0000"
#endif
#ifndef COMPILE_TIME
#define COMPILE_TIME "0000-00-00 00:00:00 UTC"
#endif

namespace fs = std::filesystem;

#if defined(_WIN32) && !defined(GUI)
bool check_requirements(std::vector<fs::path> reqs)
{
    uint8_t err_count = 0;
    for (const fs::path &path : reqs) {
        if (!fs::exists(path)) {
            std::cerr << "ERROR: " << path.filename() << " is missing!\n";
            err_count = err_count + 1;
        }
    }
    if (err_count > 0) {
        return false;
    }
    return true;
}
#endif

Cia_File fix_cia(const fs::path &path, const Settings &set)
{
    Cia_File res = {path, false, ""};
    try {
        res.result = Game(path, set).fix_banner();
    } catch (const std::system_error &e) { // this happens if e.g. the console is set to russian codepage and the cia path contains an accent somewhere
        res.message = e.what();
        res.message.append("\nSometimes this happens if your OS is set to a language other than English and the cia path contains accents or other special characters "
                           "(Both the full path to the folder the .cia file is in as well as the file itself). "
                           "If this is the case, please try renaming and moving the .cia file to a location without these characters, e.g. \"C:/\" and running again there.");
        res.path = fs::relative(fs::current_path() / "path" / "with" / "problems");
        res.result = false;
    }

    return res;
}

#if defined(_WIN32) && defined(GUI)
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    int argc = __argc;
    char** argv = __argv;
#else
int main(int argc, char* argv[])
{
#endif

    Settings set;
    set.bin = PathFind::FindExecutable();
    set.cwd = fs::current_path();
    set.out = fs::current_path() / "out";

#ifdef _WIN32
    set.dstool = set.bin.parent_path() / "tools" / "3dstool.exe";
    set.ctrtool = set.bin.parent_path() / "tools" / "ctrtool.exe";
    set.makerom = set.bin.parent_path() / "tools" / "makerom.exe";
#ifndef GUI
    if (!check_requirements(std::vector<fs::path> {set.dstool, set.ctrtool, set.makerom})) {
        std::cerr << "ERROR: requirements are missing!\n";
        return 1;
    }
#endif
#endif

#ifdef GUI
    UI a(set);
    return 0;
#else
    std::vector<fs::path> cia_paths;

    int parse_args_return = parse_args(argc, argv, cia_paths, set);
    switch (parse_args_return) {
        case 1:
            std::cerr << "ERROR: there was something wrong with the supplied arguments!\n";
            return 1;
        case 2:
            return 0;
    }

    std::vector<Cia_File> results;
    for (const auto &path : cia_paths) {
        results.push_back(fix_cia(path, set));
    }

    for (const auto &res : results) {
        if (res.result == false) {
            std::cerr << "ERROR: There was a problem processing " << res.path.string() << "\n"
                      << res.message;
        }
    }

    pause_if_double_clicked();
    return 0;
#endif
}
