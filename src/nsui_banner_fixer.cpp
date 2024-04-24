#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <subprocess.hpp>
#include <tclap/Cmdline.h>
#include <vector>

#include "Game.hpp"
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

int check_requirements(std::vector<fs::path> reqs)
{
    uint8_t err_count = 0;
    for (const fs::path &path : reqs) {
        if (!fs::exists(path)) {
            std::cerr << "ERROR: " << path.filename() << " is missing!\n";
            err_count = err_count + 1;
        }
    }
    if (err_count > 0) {
        return 1;
    }
    return 0;
}

int parse_args(int argc, char** argv, std::vector<fs::path> &cias, bool &replace, bool &verbose, bool &quiet)
{
    try {
        TCLAP::CmdLine cmd("Either supply a .cia file as an argument or place all the files you want to convert in your current working directory.", ' ', "1.4", false);
        TCLAP::UnlabeledValueArg<std::string> ciaArg("file.cia", "The .cia file to be fixed.", false, "", "path", cmd);
        TCLAP::SwitchArg versionArg("", "version", "Display the program version.", cmd, false);
        TCLAP::SwitchArg licenseArg("", "licenses", "Display license information.", cmd, false);
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
            if (!ciaArg.getValue().ends_with(".cia"))
                return 1;
            if (!fs::exists(fs::absolute(fs::path(ciaArg.getValue()))))
                return 1;
            cias.push_back(fs::absolute(fs::path(ciaArg.getValue())));
        } else {
            for (const auto &dir_entry : fs::directory_iterator(fs::current_path())) {
                if (dir_entry.path().extension().string() == ".cia")
                    cias.push_back(fs::absolute(dir_entry.path()));
            }
        }
        if (cias.size() == 0) {
            TCLAP::StdOutput().usage(cmd);
            return 1;
        }

        replace = replaceArg.getValue();
        verbose = verboseArg.getValue();
        quiet = quietArg.getValue();

    } catch (TCLAP::ArgException &e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}

int main(int argc, char* argv[])
{
    const fs::path exe = argv[0];
    dstool = exe.parent_path() / dstool;
    ctrtool = exe.parent_path() / ctrtool;
    makerom = exe.parent_path() / makerom;
    if (check_requirements(std::vector<fs::path> {dstool, ctrtool, makerom})) {
        std::cerr << "ERROR: requirements are missing!\n";
        return 1;
    }

    bool replace = false;
    bool verbose = false;
    bool quiet = false;
    std::vector<fs::path> cia_paths;

    if (parse_args(argc, argv, cia_paths, replace, verbose, quiet)) {
        std::cerr << "ERROR: there was something wrong with the supplied arguments!\n";
        return 1;
    }

    for (const auto &path : cia_paths) {
        Game(path).fix_banner(replace, verbose, quiet);
    }

    return 0;
}
