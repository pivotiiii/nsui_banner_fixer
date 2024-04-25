#include <filesystem>

#include <Game.hpp>
#include <Settings.hpp>

namespace fs = std::filesystem;

bool test_fix_banner(const fs::path &cia, const Settings &set)
{
    Game game = Game(cia, set);
    bool retval = game.fix_banner();
    fs::remove_all(fs::current_path() / "temp");
    fs::remove_all(fs::current_path() / "out");
    return retval;
}

int main(int argc, char* argv[])
{
    Settings set;
    set.replace = false;
    set.verbose = false;
    set.quiet = false;
    set.bin = argv[0];
    set.cwd = fs::current_path();
    set.dstool = fs::current_path() / "tools" / "3dstool.exe";
    set.ctrtool = fs::current_path() / "tools" / "ctrtool.exe";
    set.makerom = fs::current_path() / "tools" / "makerom.exe";

    int retval = 0;

    if (test_fix_banner(fs::current_path() / "test_v28.cia", set) != true) {
        retval = 1;
    }

    if (test_fix_banner(fs::current_path() / "test_v27.cia", set) != false) {
        retval = 2;
    }

    fs::copy_file(fs::current_path() / "test_v28.cia", fs::current_path() / "test_v28REP.cia");
    set.replace = true;
    if (test_fix_banner(fs::current_path() / "test_v28REP.cia", set) != true) {
        retval = 3;
    }
    set.replace = false;
    fs::remove_all(fs::current_path() / "test_v28REP.cia");

    return retval;
}
