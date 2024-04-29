#include <filesystem>
#include <iostream>
#include <vector>

#include <tclap/CmdLine.h>

#include "Game.hpp"
#include "Settings.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <chrono>
#include <thread>
#endif

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

void pause_if_double_clicked(bool require_key_press = true, int sleep = 0)
{
#ifdef _WIN32
    DWORD procIDs[2];
    DWORD maxCount = 2;
    DWORD result = GetConsoleProcessList((LPDWORD) procIDs, maxCount);
    if (result == 1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        if (require_key_press) {
            system("pause");
        }
    }
#endif
}

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

int parse_args(int argc, char** argv, std::vector<fs::path> &cias, Settings &set)
{
    try {
        TCLAP::CmdLine cmd("Either supply a .cia file as an argument or place all the files you want to convert in your current working directory and run this program again.", ' ', "1.4", false);
        TCLAP::UnlabeledValueArg<std::string> ciaArg("file.cia", "The .cia file to be fixed.", false, "", "path", cmd);
        TCLAP::SwitchArg replaceArg("r", "replace", "Fix the .cia file(s) directly instead of saving a fixed copy in /out", cmd, false);
        TCLAP::SwitchArg verboseArg("v", "verbose", "Display more output as the program is working.", cmd, false);
        TCLAP::SwitchArg quietArg("q", "quiet", "Silence any non error output.", cmd, false);
        TCLAP::SwitchArg helpArg("h", "help", "Display this help message.", cmd, false);
        TCLAP::SwitchArg versionArg("", "version", "Display the program version.", cmd, false);
        TCLAP::SwitchArg licenseArg("", "licenses", "Display license information.", cmd, false);

        cmd.parse(argc, argv);

        if (versionArg) {
            std::cout << "nsui_banner_fixer " << VERSION << "\nCopyright (c) " << YEAR << " pivotiii\nbuilt " << COMPILE_TIME << "\n";
            return 2;
        }

        if (licenseArg) {
            std::cout << "nsui_banner_fixer " << VERSION << "\nCopyright (c) " << YEAR << " pivotiii\n\n"
                                                                                          "nsui_banner_fixer uses the following tools and libraries:\n\n"
                                                                                          "CTRTOOL\nCopyright (c) 2016 neimod, 3DSGuy\n\n"
                                                                                          "CTR MAKEROM v0.15\nCopyright (c) 2014 3DSGuy\n\n"
                                                                                          "nsui_banner_fixer uses the following tools and libraries licensed under the MIT license:\n\n"
                                                                                          "3dstool\nCopyright (c) 2014-2020 Daowen Sun\n\n"
                                                                                          "TCLAP\nCopyright (c) 2003 Michael E. Smoot\nCopyright (c) 2004 Daniel Aarno\nCopyright (c) 2017 Google Inc.\n\n"
                                                                                          "The full MIT license text is available at https://github.com/pivotiiii/nsui_banner_fixer/blob/master/LICENSE\n";
            return 2;
        }

        if (ciaArg.getValue() != "") {
            if (!ciaArg.getValue().ends_with(".cia")) {
                std::cerr << "ERROR: the supplied file is not a .cia file!\n";
                return 1;
            }
            if (!fs::exists(fs::absolute(fs::path(ciaArg.getValue())))) {
                std::cerr << "ERROR: cannot find the specified .cia file! (" << ciaArg.getValue() << ")\n";
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
            pause_if_double_clicked(false, 100); // TCLAP output gets corrupted otherwise :(
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

#ifdef _WIN32
    set.dstool = set.bin.parent_path() / "tools" / "3dstool.exe";
    set.ctrtool = set.bin.parent_path() / "tools" / "ctrtool.exe";
    set.makerom = set.bin.parent_path() / "tools" / "makerom.exe";
    if (!check_requirements(std::vector<fs::path> {set.dstool, set.ctrtool, set.makerom})) {
        std::cerr << "ERROR: requirements are missing!\n";
        return 1;
    }
#endif

    std::vector<fs::path> cia_paths;

    int parse_args_return = parse_args(argc, argv, cia_paths, set);
    switch (parse_args_return) {
        case 1:
            std::cerr << "ERROR: there was something wrong with the supplied arguments!\n";
            return 1;
        case 2:
            return 0;
    }

    struct resultS {
        fs::path cia;
        bool result;
        std::string message;
    };
    std::vector<struct resultS> results;

    for (const auto &path : cia_paths) {
        struct resultS res = {path, false};
        try {
            res.result = Game(path, set).fix_banner();
        } catch (const std::system_error &e) { // this happens if e.g. the console is set to russian codepage and the cia path contains an accent somewhere
            res.message = e.what();
            res.message.append("\nSometimes this happens if your OS is set to a language other than English and the cia path contains accents or other special characters "
                               "(Both the full path to the folder the .cia file is in as well as the file itself). "
                               "If this is the case, please try renaming and moving the .cia file to a location without these characters, e.g. \"C:/\" and running again there.");
            res.cia = fs::relative(fs::current_path() / "path" / "with" / "problems");
            res.result = false;
        }

        results.push_back(res);
    }

    for (const auto &res : results) {
        if (res.result == false) {
            std::cerr << "ERROR: There was a problem processing " << res.cia.string() << "\n"
                      << res.message;
        }
    }

    pause_if_double_clicked();
    return 0;
}
