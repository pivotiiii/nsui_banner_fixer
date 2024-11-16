#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <chrono>
#include <thread>
#endif

#include <tclap/CmdLine.h>

#include "ArgsParser.hpp"

namespace fs = std::filesystem;

bool get_cia_files(std::string ciaArg, std::vector<fs::path> &cias)
{
    if (ciaArg != "") {
        if (!ciaArg.ends_with(".cia")) {
            std::cerr << "ERROR: the supplied file is not a .cia file!\n";
            return false;
        }
        if (!fs::exists(fs::absolute(fs::path(ciaArg)))) {
            std::cerr << "ERROR: cannot find the specified .cia file! (" << ciaArg << ")\n";
            return false;
        }
        cias.push_back(fs::absolute(fs::path(ciaArg)));
    } else {
        for (const auto &dir_entry : fs::directory_iterator(fs::current_path())) {
            if (dir_entry.path().extension().string() == ".cia") {
                cias.push_back(fs::absolute(dir_entry.path()));
            }
        }
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

        if (!get_cia_files(ciaArg.getValue(), cias)) {
            return 1;
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
