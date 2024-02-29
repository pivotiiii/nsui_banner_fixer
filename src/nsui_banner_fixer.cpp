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

        int fix_banner(bool replace = false, bool verbose = false)
        {
            this->extract_cia(verbose);
            this->edit_bcmdl(verbose);
            this->repack_cia(replace, verbose);
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
            return version;
        }

        int extract_cia(bool verbose = false)
        {
            std::vector<std::string> extract_contents = {ctrtool.string(),
                            std::string("--contents=") + (this->cwd / "contents").string(), 
                            this->cia_path.string()};
            for (auto i : extract_contents)
                std::cout << i << " ";
            if (sp::call(extract_contents))
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

        int edit_bcmdl(bool verbose = false)
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

        int repack_cia(bool replace = false, bool verbose = false)
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
                                                        "exefs", "-f", (this->cwd / "exefs.bin").string(),
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

            fs::path out_cia;
            if (replace){
                out_cia = this->cia_path;
            }
            else {
                fs::path out_dir = this->cwd.parent_path().parent_path() / "out";
                fs::create_directories(out_dir);
                out_cia = out_dir / (this->name  + ".cia");
            }
            fs::path content_path_rel = fs::relative(this->cwd / (this->name + ".cxi"), fs::current_path());
            
            std::vector<std::string> rebuild_cia = {makerom.string(),
                                                    "-f", "cia",
                                                    "-o", out_cia.string(),
                                                    "-content", content_path_rel.string() + ":0:0x00",
                                                    "-major", std::to_string(this->version.major),
                                                    "-minor", std::to_string(this->version.minor),
                                                    "-micro", std::to_string(this->version.micro)};
            if (sp::call(rebuild_cia))
                return 1;
            
            return 0;
        }

};

int check_requirements(std::vector<fs::path> reqs)
{
    uint8_t err_count = 0;
    for (fs::path const& path : reqs) {
        if (!fs::exists(path)){
            std::cerr << "ERROR: " << path.filename() << " is missing!\n";
            err_count = err_count + 1;
        }
    }
    if (err_count > 0){
        return 1;
    }
    return 0;
}

int parse_args(int argc, char** argv, std::vector<fs::path> &cias, bool &replace, bool &verbose)
{
    try
    {
        TCLAP::CmdLine cmd("Either supply a .cia file as an argument or place all the files you want to convert in your current working directory.", ' ', "1.4", false);
        TCLAP::UnlabeledValueArg<std::string> ciaArg("file.cia", "The .cia file to be fixed.", false, "", "path", cmd);
        TCLAP::SwitchArg replaceArg("r", "replace", "Set this flag to fix the .cia file(s) directly instead of saving a fixed copy in /out", cmd, false);
        TCLAP::SwitchArg verboseArg("v", "verbose", "Set this flag to see more output as the program is working.", cmd, false);
        cmd.parse(argc, argv);

        if (ciaArg.getValue() != ""){
            if (!ciaArg.getValue().ends_with(".cia"))
                return 1;
            if (!fs::exists(fs::absolute(fs::path(ciaArg.getValue()))))
                return 1;
            cias.push_back(fs::absolute(fs::path(ciaArg.getValue())));
        }
        else {
            for (auto const& dir_entry : fs::directory_iterator(fs::current_path())){
                if (dir_entry.path().extension().string() == ".cia")
                    cias.push_back(fs::absolute(dir_entry.path()));
            }
        }
        if (cias.size() == 0){
            TCLAP::StdOutput().usage(cmd);
        }

        replace = replaceArg.getValue();
        verbose = verboseArg.getValue();
        
    }
    catch(TCLAP::ArgException &e)
    {
        std::cerr << e.what() << '\n';
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

    if (check_requirements(std::vector<fs::path>{dstool, ctrtool, makerom})){
        std::cerr << "ERROR: requirements are missing!\n";
        return 1;
    }
        
    bool replace = false;
    bool verbose = false;
    std::vector<fs::path> cia_paths;

    if (parse_args(argc, argv, cia_paths, replace, verbose)){
        std::cerr << "ERROR: there was something wrong with the supplied arguments!\n";
        return 1;
    }

    if (cia_paths.size() == 0){
        
    }

    for (auto const& path : cia_paths){
        std::cout << path.string();
        Game(path).fix_banner(replace, verbose);
    }

    //std::cout << "pre init game\n";
    //Game test = Game(fs::path("D:/Documents/VS Code/nsui_banner_fixer/cpp/src/Castlevania Aria of Sorrow.cia"));
    //test.fix_banner();
    
    //std::vector<Game> cias;
    //parseArgs(&cias, &replace, &verbose);
    
    return 0;
}