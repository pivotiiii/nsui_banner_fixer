#ifdef _WIN32
#include <Windows.h>
#endif

#include <CLI11.hpp>

#include "ArgsParser.hpp"

namespace fs = std::filesystem;

bool get_cia_files(fs::path ciaArg, std::vector<fs::path> &cias)
{
    if (fs::exists(ciaArg)) {
        if (ciaArg.extension().string() != ".cia") {
            std::cerr << "ERROR: the supplied file is not a .cia file!\n";
            return false;
        }
        if (!fs::exists(fs::absolute(ciaArg))) {
            std::cerr << "ERROR: cannot find the specified .cia file! (" << ciaArg << ")\n";
            return false;
        }
        cias.push_back(fs::absolute(ciaArg));
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
    CLI::App app("Either supply a .cia file as an argument or place all the files you want to convert in your current working directory and run this program again.");
    app.name("nsui_banner_fixer.exe");
    fs::path ciaArg = "";
    app.add_option("file.cia", ciaArg, "The path to the .cia file to be fixed.");
    bool replaceArg = false;
    app.add_flag("-r,--replace", replaceArg, "Fix the .cia file(s) directly instead of saving a fixed copy in /out.");
    bool verboseArg = false;
    app.add_flag("-v,--verbose", verboseArg, "Display more output as the program is working.");
    bool quietArg = false;
    app.add_flag("-q,--quiet", quietArg, "Silence any non error output.");
    app.set_help_flag("");
    app.set_help_all_flag("");
    bool helpArg = false;
    app.add_flag("-h,--help", helpArg, "Display this help message.");
    bool versionArg = false;
    app.add_flag("--version", versionArg, "Display the program version.");
    bool licenseArg = false;
    app.add_flag("--licenses", licenseArg, "Display license information.");

    argv = app.ensure_utf8(argv);
    CLI11_PARSE(app, argc, argv);

    if (versionArg) {
        std::cout << "nsui_banner_fixer " << VERSION << "\nCopyright (c) " << YEAR << " pivotiii\nbuilt " << COMPILE_TIME << "\n";
        return 2;
    }

    if (licenseArg) {
        std::cout << "nsui_banner_fixer " << VERSION << "\nCopyright (c) " << YEAR << " pivotiii\nFull license info is available at https://raw.githubusercontent.com/pivotiiii/nsui_banner_fixer/refs/heads/master/LICENSE\n";
        return 2;
    }

    if (!get_cia_files(ciaArg, cias)) {
        return 1;
    }

    if (cias.size() == 0 || helpArg) {
#ifdef _WIN32
        pause_if_double_clicked(false, 100); // output gets corrupted otherwise :(
        std::cout << app.help() << std::endl;
        pause_if_double_clicked();
#else
        std::cout << app.help() << std::endl;
#endif
        return 2;
    }

    set.replace = replaceArg;
    set.verbose = verboseArg;
    set.quiet = quietArg;

    if (set.quiet && set.verbose) {
        set.quiet = false;
    }

    return 0;
}
