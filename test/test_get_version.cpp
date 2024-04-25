#include <filesystem>
#include <regex>

#include <subprocess.hpp>

#include <Game.hpp>
#include <Settings.hpp>

namespace fs = std::filesystem;

versionS get_version(const fs::path &cia, const Settings set)
{
    versionS version;
    auto output_buffer = subprocess::check_output({set.ctrtool.string(), "-i", cia.string()});
    if (output_buffer.buf.data() != NULL) {
        std::stringstream ss(output_buffer.buf.data());
        std::string line;
        while (std::getline(ss, line, '\n')) {
            if (line.starts_with("Title version:")) {
                std::regex version_regex("(\\d+).(\\d+).(\\d+)");
                std::smatch matches;
                if (std::regex_search(line, matches, version_regex)) {
                    if (matches.size() == 4) {
                        version.major = max(min(63, stoi(matches[1].str())), 0);
                        version.minor = max(min(63, stoi(matches[2].str())), 0);
                        version.micro = max(min(15, stoi(matches[3].str())), 0);
                    }
                }
                break;
            }
        }
    }
    return version;
}

bool test_get_version(const fs::path &cia, const Settings &set)
{
    Game game = Game(cia, set);
    game.fix_banner();

    versionS version = get_version(cia, set);

    fs::remove_all(fs::current_path() / "temp");
    fs::remove_all(fs::current_path() / "out");

    if (version.major == 0 && version.minor == 6 && version.micro == 9) {
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
