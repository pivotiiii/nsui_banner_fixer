#include <iostream>
#include <sstream>
#include <vector>
#include <regex>
#include <filesystem>
#include "nsui_banner_fixer.hpp"
#include <fstream>
#include <tclap/Cmdline.h>
#include <subprocess.hpp>

namespace fs = std::filesystem;
namespace sp = subprocess;

/*const char locale_codes[13][6] = {
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
        {'U', 'S', 'A', '_', 'P', 'O'}};*/

const std::vector<std::string> locale_codes = {
        "EUR_EN",
        "EUR_FR",
        "EUR_GE",
        "EUR_IT",
        "EUR_SP",
        "EUR_DU",
        "EUR_PO",
        "EUR_RU",
        "JPN_JP",
        "USA_EN",
        "USA_FR",
        "USA_SP",
        "USA_PO"};

const std::vector<int> locale_offsets = {0x14BC, 0x14CB};

fs::path dstool = fs::path("tools") / "3dstool.exe";
fs::path ctrtool = fs::path("tools") / "ctrtool.exe";
fs::path makerom = fs::path("tools") / "makerom.exe";

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
            this->name = cia_path.stem().string();
            this->version = this->get_version();
            this->cwd = fs::current_path() / "temp" / this->name;
            fs::create_directories(this->cwd);
        }

        ~Game()
        {
            fs::remove_all(this->cwd.parent_path());
        }

        int fix_banner()
        {
            this->extract_cia();
            this->edit_bcmdl();
            return 0;
        }

        

    private:
        const fs::path cia_path;
        fs::path cwd;
        std::string name;
        std::string banner_ext = "bin";
        versionS version;

        versionS get_version()
        {
            versionS version;
            
            auto output_buffer = sp::check_output({"D:/Documents/VS Code/nsui_banner_fixer/cpp/src/tools/ctrtool.exe", "-i", this->cia_path.string()});

            if (output_buffer.buf.data() != NULL){
                std::stringstream ss(output_buffer.buf.data());
                std::string line;
                while (std::getline(ss, line, '\n')){
                    if (line.starts_with("Title version:")){
                        std::cout << line << std::endl;

                        std::regex version_regex("(\\d+).(\\d+).(\\d+)");
                        std::smatch matches;
                        if (std::regex_search(line, matches, version_regex)){
                            if (matches.size() == 4){
                                version.major = max(min(63, stoi(matches[1].str())), 0);
                                version.minor = max(min(63, stoi(matches[2].str())), 0);
                                version.micro = max(min(15, stoi(matches[3].str())), 0);
                            }
                        }
                        break;
                    }    
                }
            }
            std::cout << version.major << version.minor << version.micro << std::endl;
            return version;
        }

        int extract_cia()
        {
            if (sp::call({ctrtool.string(), 
                            std::string("--contents=") + (this->cwd / "contents").string(), 
                            this->cia_path.string()}))
                return 1;
            
            if (sp::call({dstool.string(),
                            "-x", "-t",
                            "cxi", "-f", (this->cwd / "contents.0000.00000000").string(),
                            "--header", (this->cwd / "ncch.header").string(),
                            "--exh", (this->cwd / "exheader.bin").string(),
                            "--exefs", (this->cwd / "exefs.bin").string(),
                            "--romfs", (this->cwd / "romfs.bin").string()}))
                return 1;
                            
            if (sp::call({dstool.string(), 
                            "-x", "-t", 
                            "exefs", "-f", (this->cwd / "exefs.bin").string(),
                            "--header", (this->cwd / "exefs.header").string(),
                            "--exefs-dir", (this->cwd / "exefs").string()}))
                return 1;
            
            fs::create_directory(this->cwd / "banner");
            if (fs::exists(this->cwd / "exefs" / "banner.bnr"))
                this->banner_ext = "bnr";

            if (sp::call({dstool.string(), 
                            "-x", "-t", 
                            "banner", "-f", (this->cwd / "exefs" / (std::string("banner.") + this->banner_ext)).string(),
                            "--banner-dir", (this->cwd / "banner").string()}))
                return 1;
            
            return 0;
        }

        int edit_bcmdl()
        {
            for (int i = 1; i < 14; i++){
                std::fstream file;
                file.open((this->cwd / "banner" / (std::string("banner") + std::to_string(i) + ".bcmdl")).string(), 
                            std::fstream::in | std::fstream::out | std::fstream::binary);
                if (file.is_open()){
                    char rbuf[10] = {0};
                    for (auto const& offset : locale_offsets){
                        file.seekg(offset, std::fstream::beg);
                        file.read(rbuf, 6);
                        std::cout << rbuf << "\n";
                        if (!strncmp(rbuf, locale_codes[i-1].c_str(), 6) && !strncmp(rbuf, "USA_EN", 6))
                            return 1;
                        file.seekp(offset, std::fstream::beg);
                        file.write(locale_codes[i-1].c_str(), 6);
                    }
                }
                file.close();
            }
            return 0;
        }

        int repack_cia()
        {
            fs::remove(this->cwd / "exefs" / (std::string("banner.") + this->banner_ext));
            std::vector<std::string> rebuild_banner = {dstool.string(),
                                                        "-c", "-t",
                                                        "banner", "-f", (this->cwd / "exefs" / (std::string("banner.") + this->banner_ext)).string(),
                                                        "--banner-dir", (this->cwd / "banner").string()};
            if (sp::call(rebuild_banner))
                return 1;
            
            std::vector<std::string> rebuild_exefs = {dstool.string(),
                                                        "-c", "-t",
                                                        "exefs", (this->cwd / "exefs.bin").string(),
                                                        "--header", (this->cwd / "exefs.header").string(),
                                                        "--exefs-dir", (this->cwd / "exefs").string()};
            if (sp::call(rebuild_exefs))
                return 1;
            
            std::vector<std::string> rebuild_cxi = {dstool.string(),
                                                    "-c", "-t",
                                                    "cxi", "-f", (this->cwd / (this->name + ".cxi")).string(),
                                                    "--header", (this->cwd / "ncch.header").string(),
                                                    "--exh", (this->cwd / "exheader.bin").string(),
                                                    "--exefs", (this->cwd / "exefs.bin").string(),
                                                    "--romfs", (this->cwd / "romfs.bin").string()};
            if (sp::call(rebuild_cxi))
                return 1;

            
            
        }

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
    //const fs::path dstool = exe.parent_path() / "tools" / "3dstool.exe";
    //const fs::path ctrtool = exe.parent_path() / "tools" / "ctrtool.exe";
    //const fs::path makerom = exe.parent_path() / "tools" / "makerom.exe";
    dstool = exe.parent_path() / dstool;
    ctrtool = exe.parent_path() / ctrtool;
    makerom = exe.parent_path() / makerom;

    std::string a = "EUR_EN";
    std::string c = "X";
    char b[6] = {'E', 'U', 'R', '_', 'E', 'N'};
    std::cout << a << " " << sizeof(a) << std::endl;
    std::cout << a[0] << " " << sizeof(a[0]) << std::endl;
    std::cout << a.substr(0, 4) << " " << sizeof(a.substr(0, 4)) << std::endl;
    std::cout << a.data() << " " << sizeof(a.data()) << std::endl;
    std::cout << b << " " << sizeof(b) << std::endl;
    //char j[6];
    //strncpy(j, locale_codes[3], 6);
    //std::cout << j << " " << sizeof(j) << std::endl;

    std::fstream file;
    file.open("test.txt", std::fstream::in | std::fstream::out | std::fstream::binary);
    if (file.is_open())
    {
        file.seekg(3, std::fstream::beg);
        char buf[20];
        file.read(buf, 1);
        std::cout << std::endl << buf << std::endl;
        file.seekg(0, std::fstream::beg);
        file.write(locale_codes[4].c_str(), sizeof(locale_codes[4]));
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
    std::cout << "pre init game\n";
    Game test = Game(fs::path("D:/Documents/VS Code/nsui_banner_fixer/cpp/src/Castlevania Aria of Sorrow.cia"));
    test.fix_banner();
    
    std::vector<Game> cias;
    //parseArgs(&cias, &replace, &verbose);
    
    return 0;
}