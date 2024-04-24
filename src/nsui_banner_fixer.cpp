#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <subprocess.hpp>
#include <tclap/Cmdline.h>
#include <vector>

#include "Game.hpp"
#include "Settings.hpp"

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
namespace sp = subprocess;

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

void pause_if_double_clicked()
{
    DWORD procIDs[2];
    DWORD maxCount = 2;
    DWORD result = GetConsoleProcessList((LPDWORD) procIDs, maxCount);
    if (result == 1) {
        system("pause");
    }
}

int parse_args(int argc, char** argv, std::vector<fs::path> &cias, Settings &set)
{
    try {
        TCLAP::CmdLine cmd("Either supply a .cia file as an argument or place all the files you want to convert in your current working directory and run this program again.", ' ', "1.4", false);
        TCLAP::UnlabeledValueArg<std::string> ciaArg("file.cia", "The .cia file to be fixed.", false, "", "path", cmd);
        TCLAP::SwitchArg replaceArg("r", "replace", "Fix the .cia file(s) directly instead of saving a fixed copy in /out", cmd, false);
        TCLAP::SwitchArg verboseArg("v", "verbose", "Display more output as the program is working.", cmd, false);
        TCLAP::SwitchArg quietArg("q", "quiet", "Silence any non error output.", cmd, false);
        TCLAP::SwitchArg versionArg("", "version", "Display the program version.", cmd, false);
        TCLAP::SwitchArg licenseArg("", "licenses", "Display license information.", cmd, false);
        TCLAP::SwitchArg helpArg("h", "help", "Display this help message.", cmd, false);

        cmd.parse(argc, argv);

        if (versionArg) {
            std::cout << "nsui_banner_fixer " << VERSION << "\nCopyright (c) " << YEAR << " pivotiii\nbuilt " << COMPILE_TIME << "\n";
            return 2;
        }

        if (licenseArg) {
            std::cout << "nsui_banner_fixer " << VERSION << "\nCopyright (c) " << YEAR << " pivotiii\n\n"
                                                                                          "nsui_banner_fixer uses the following libraries licensed under the MIT license:\n\n"
                                                                                          "--- TCLAP ---\nCopyright (c) 2003 Michael E. Smoot\nCopyright (c) 2004 Daniel Aarno\nCopyright (c) 2017 Google Inc.\n\n"
                                                                                          "--- cpp-subprocess ---\nCopyright (c) 2016-2018 Arun Muralidharan\n\n"
                                                                                          "The full license text is available at https://github.com/pivotiiii/nsui_banner_fixer/blob/master/LICENSE\n";
            return 2;
        }

        if (ciaArg.getValue() != "") {
            if (!ciaArg.getValue().ends_with(".cia")) {
                std::cerr << "ERROR: the supplied file is not a .cia file!\n";
                return 1;
            }
            if (!fs::exists(fs::absolute(fs::path(ciaArg.getValue())))) {
                std::cerr << "ERROR: cannot find the specified .cia file!\n";
                return 1;
            }
            cias.push_back(fs::absolute(fs::path(ciaArg.getValue())));
        } else {
            for (const auto &dir_entry : fs::directory_iterator(fs::current_path())) {
                if (dir_entry.path().extension().string() == ".cia") {
                    cias.push_back(fs::absolute(dir_entry.path()));
                }
            }
        }
        if (cias.size() == 0 || helpArg) {
            TCLAP::StdOutput().usage(cmd);
            pause_if_double_clicked();
            return 2;
        }

        set.replace = replaceArg.getValue();
        set.verbose = verboseArg.getValue();
        set.quiet = quietArg.getValue();

        if (set.quiet && set.verbose) {
            set.quiet = false;
        }

    } catch (TCLAP::ArgException &e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}

int main(int argc, char* argv[])
{
    Settings set;
    set.bin = argv[0];
    set.cwd = fs::current_path();

    set.dstool = set.bin.parent_path() / "tools" / "3dstool.exe";
    set.ctrtool = set.bin.parent_path() / "tools" / "ctrtool.exe";
    set.makerom = set.bin.parent_path() / "tools" / "makerom.exe";
    if (!check_requirements(std::vector<fs::path> {set.dstool, set.ctrtool, set.makerom})) {
        std::cerr << "ERROR: requirements are missing!\n";
        return 1;
    }

    std::vector<fs::path> cia_paths;

    int parse_args_return = parse_args(argc, argv, cia_paths, set);
    switch (parse_args_return) {
        case 1:
            std::cerr << "ERROR: there was something wrong with the supplied arguments!\n";
            return 1;
        case 2:
            return 0;
    }

    for (const auto &path : cia_paths) {
        Game(path, set).fix_banner();
    }

    return 0;
}
