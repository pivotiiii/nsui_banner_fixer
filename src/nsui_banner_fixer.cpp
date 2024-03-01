#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <subprocess.hpp>
#include <tclap/Cmdline.h>
#include <vector>

#include "Game.hpp"
#include "globals.hpp"
#include "nsui_banner_fixer.hpp"

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
        TCLAP::SwitchArg replaceArg("r", "replace", "Set this flag to fix the .cia file(s) directly instead of saving a fixed copy in /out", cmd, false);
        TCLAP::SwitchArg verboseArg("v", "verbose", "Set this flag to see more output as the program is working.", cmd, false);
        TCLAP::SwitchArg quietArg("q", "quiet", "Set this flag to silence any non error output.", cmd, false);
        cmd.parse(argc, argv);

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
