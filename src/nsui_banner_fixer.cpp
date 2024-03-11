#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <subprocess.hpp>
#include <tclap/CmdLine.h>
#include <vector>

#include "Game.hpp"
#include "Settings.hpp"
#include "globals.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "stdafx.h"
#include <Windows.h>
#endif

namespace fs = std::filesystem;
namespace sp = subprocess;

void pause_if_double_clicked()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    DWORD procIDs[2];
    DWORD maxCount = 2;
    DWORD result = GetConsoleProcessList((LPDWORD) procIDs, maxCount);
    if (result == 1) {
        system("pause");
    }
#endif
}

bool parse_args(int argc, char** argv, std::vector<fs::path> &cias, Settings &set)
{
    try {
        TCLAP::CmdLine cmd("Either supply a .cia file as an argument or place all the files you want to convert in your current working directory.", ' ', "1.4", false);
        TCLAP::UnlabeledValueArg<std::string> ciaArg("file.cia", "The .cia file to be fixed.", false, "", "path", cmd);
        TCLAP::SwitchArg replaceArg("r", "replace", "Set this flag to fix the .cia file(s) directly instead of saving a fixed copy in /out", cmd, false);
        TCLAP::SwitchArg verboseArg("v", "verbose", "Set this flag to see more output as the program is working.", false);
        TCLAP::SwitchArg quietArg("q", "quiet", "Set this flag to silence any non error output.", false);

        TCLAP::EitherOf verbosity;
        verbosity.add(verboseArg);
        verbosity.add(quietArg);
        cmd.add(verbosity);

        cmd.parse(argc, argv);

        if (ciaArg.getValue() != "") {
            if (!ciaArg.getValue().ends_with(".cia"))
                return false;
            if (!fs::exists(fs::absolute(fs::path(ciaArg.getValue()))))
                return false;
            cias.push_back(fs::absolute(fs::path(ciaArg.getValue())));
        } else {
            for (const auto &dir_entry : fs::directory_iterator(fs::current_path())) {
                if (dir_entry.path().extension().string() == ".cia")
                    cias.push_back(fs::absolute(dir_entry.path()));
            }
        }
        if (cias.size() == 0) {
            TCLAP::StdOutput().usage(cmd);
            return false;
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

    return true;
}
// int main_3dstool(int argc, char* argv[]);
int main(int argc, char* argv[])
{
    Settings set;
    set.bin = argv[0];
    set.cwd = fs::current_path();

    std::vector<fs::path> cia_paths;

    if (!parse_args(argc, argv, cia_paths, set)) {
        std::cerr << "ERROR: there was something wrong with the supplied arguments!\n";
        return 1;
    }

    struct result {
        fs::path cia;
        bool result;
    };
    std::vector<struct result> results;

    for (const auto &path : cia_paths) {
        struct result res = {path, false};
        res.result = Game(path, set).fix_banner();
        results.push_back(res);
    }

    for (const auto &res : results) {
        if (res.result == false) {
            std::cerr << "ERROR: There was a problem processing " << res.cia << "\n";
        }
    }

    pause_if_double_clicked();
    return 0;
}
