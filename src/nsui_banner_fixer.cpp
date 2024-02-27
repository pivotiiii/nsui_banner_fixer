#include <iostream>
#include <vector>
#include <regex>
#include <filesystem>
#include "nsui_banner_fixer.hpp"
#include <fstream>
#include <tclap/Cmdline.h>
#include <subprocess.hpp>

namespace fs = std::filesystem;
namespace sp = subprocess;

const char locale_codes[13][6] = {
        {'E', 'U', 'R', '_', 'E', 'N'},
        {'E', 'U', 'R', '_', 'F', 'R'},
        {'E', 'U', 'R', '_', 'G', 'E'},
        {'E', 'U', 'R', '_', 'I', 'T'},
        {'E', 'U', 'R', '_', 'S', 'P'},
        {'E', 'U', 'R', '_', 'D', 'U'},
        {'E', 'U', 'R', '_', 'P', 'O'},
        {'E', 'U', 'R', '_', 'R', 'U'},
        {'J', 'P', 'N', '_', 'J', 'P'},
        {'U', 'S', 'A', '_', 'E', 'N'},
        {'U', 'S', 'A', '_', 'F', 'R'},
        {'U', 'S', 'A', '_', 'S', 'P'},
        {'U', 'S', 'A', '_', 'P', 'O'}};

const std::vector<int> locale_offsets = {0x14BC, 0x14CB};

typedef struct versionS{
    int major = 0;
    int minor = 0;
    int micro = 0;
} versionS;

class Game
{
    public:
        Game(fs::path cia) : cia_path(cia)
        {
            this->name = cia_path.stem();
            this->version = this->get_version();
        }

        versionS get_version()
        {
            versionS version;
            auto output = sp::call({"D:/Documents/VS Code/nsui_banner_fixer/cpp/src/tools/ctrtool.exe", "-i", this->cia_path});
            //std::cout << "Data: " << obuf.buf.data() << std::endl;
            return version;
        }
    private:
        const fs::path cia_path;
        std::string name;
        std::string banner_ext = "bin";
        versionS version;

};

int check_requirements(std::vector<fs::path> reqs)
{
    for (fs::path const& path : reqs) {
        if (!fs::exists(path))
            std::cout << "ERROR: " << path.filename() << " is missing!\n";
            return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    const fs::path exe = argv[0];
    const fs::path dstool = exe.parent_path() / "tools" / "3dstool.exe";
    const fs::path ctrtool = exe.parent_path() / "tools" / "ctrtool.exe";
    const fs::path makerom = exe.parent_path() / "tools" / "makerom.exe";

    std::string a = "EUR_EN";
    std::string c = "X";
    char b[6] = {'E', 'U', 'R', '_', 'E', 'N'};
    std::cout << a << " " << sizeof(a) << std::endl;
    std::cout << a[0] << " " << sizeof(a[0]) << std::endl;
    std::cout << a.substr(0, 4) << " " << sizeof(a.substr(0, 4)) << std::endl;
    std::cout << a.data() << " " << sizeof(a.data()) << std::endl;
    std::cout << b << " " << sizeof(b) << std::endl;
    char j[6];
    strncpy(j, locale_codes[3], 6);
    std::cout << j << " " << sizeof(j) << std::endl;

    std::fstream file;
    file.open("test.txt", std::fstream::in | std::fstream::out | std::fstream::binary);
    if (file.is_open())
    {
        file.seekg(3, std::fstream::beg);
        char buf[20];
        file.read(buf, 1);
        std::cout << std::endl << buf << std::endl;
        file.seekg(0, std::fstream::beg);
        file.write(locale_codes[4], sizeof(locale_codes[4]));
        file.seekg(0, std::fstream::beg);
        file.read(buf, 10);
        std::cout << std::endl << buf << std::endl;
        
    }
    std::cout << "argv[0]: " << argv[0] << "\n";
    std::cout << "cwd: " << fs::current_path() << "\n";
    if (!check_requirements(std::vector<fs::path>{dstool, ctrtool, makerom}))
        return 1;
    
    bool replace = false;
    bool verbose = false;
    Game test = Game(fs::path("D:/Documents/VS Code/nsui_banner_fixer/cpp/src/Castlevania Aria of Sorrow.cia"));
    
    std::vector<Game> cias;
    //parseArgs(&cias, &replace, &verbose);
    
    return 0;
}