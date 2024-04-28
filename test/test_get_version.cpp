#include <filesystem>

#include <Game.hpp>
#include <Settings.hpp>

#if defined(_WIN32)
#include <Boost/process.hpp>
#include <regex>
namespace bp = boost::process;
#elif defined(__linux)
#include <Tool.hpp>
#endif

namespace fs = std::filesystem;

versionS get_version(const fs::path &cia, const Settings set)
{
#if defined(_WIN32)
    versionS version;
    bp::ipstream output_stream;
    std::string line;

    bp::system(bp::exe = set.ctrtool.string(),
               bp::args = {"-i",
                           cia.string()},
               bp::std_out > output_stream);

    while (std::getline(output_stream, line)) {
        if (line.starts_with("Title version:")) {
            std::regex version_regex("(\\d+).(\\d+).(\\d+)");
            std::smatch matches;
            if (std::regex_search(line, matches, version_regex)) {
                if (matches.size() == 4) {
                    version.major = std::max(std::min(63, stoi(matches[1].str())), 0);
                    version.minor = std::max(std::min(63, stoi(matches[2].str())), 0);
                    version.micro = std::max(std::min(15, stoi(matches[3].str())), 0);
                }
            }
            break;
        }
    }
    return version;
#elif defined(__linux__)
    Tool::CTR ctr(cia, set.cwd, set);
    versionS version = ctr.get_cia_version();
    return version;
#endif
}

bool test_get_version(const fs::path &cia, const Settings &set)
{
    Game game = Game(cia, set);
    game.fix_banner();

    versionS version = get_version(cia, set);

    fs::remove_all(fs::current_path() / "temp");
    fs::remove_all(fs::current_path() / "out");

    if (version.major == 0 && version.minor == 1 && version.micro == 2) {
        return true;
    }
    return false;
}

bool test_get_version_before_after(const fs::path &cia, const Settings &set)
{
    Game game = Game(cia, set);

    versionS version1 = get_version(cia, set);
    game.fix_banner();
    versionS version2 = get_version((fs::current_path() / "out" / cia.filename()), set);

    fs::remove_all(fs::current_path() / "temp");
    fs::remove_all(fs::current_path() / "out");

    if (version1.major == version2.major && version1.minor == version2.minor && version1.micro == version2.micro) {
        return true;
    }
    return false;
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

    if (test_get_version(fs::current_path() / "test_v28.cia", set) != true) {
        retval = 1;
    }

    if (test_get_version_before_after(fs::current_path() / "test_v28.cia", set) != true) {
        retval = 2;
    }

    return retval;
}
