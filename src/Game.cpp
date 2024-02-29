#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <subprocess.hpp>
#include <vector>

#include "Game.hpp"
#include "globals.hpp"

namespace fs = std::filesystem;
namespace sp = subprocess;

Game::Game(fs::path cia) // : cia_path(cia)
{
    this->cia_path = cia;
    this->name = cia_path.stem().string();
    this->banner_ext = "bin";
    this->version = this->get_version();
    this->cwd = fs::current_path() / "temp" / this->name;
    fs::create_directories(this->cwd);
}

Game::~Game()
{
    fs::remove_all(this->cwd.parent_path());
}

int Game::fix_banner(bool replace = false, bool verbose = false)
{
    this->extract_cia(verbose);
    this->edit_bcmdl(verbose);
    this->repack_cia(replace, verbose);
    return 0;
}

versionS Game::get_version()
{
    versionS version;

    auto output_buffer = sp::check_output({"D:/Documents/VS Code/nsui_banner_fixer/cpp/src/tools/ctrtool.exe", "-i", this->cia_path.string()});
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

int Game::extract_cia(bool verbose = false)
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

int Game::edit_bcmdl(bool verbose = false)
{
    for (int i = 1; i < 14; i++) {
        std::fstream file;
        file.open((this->cwd / "banner" / (std::string("banner") + std::to_string(i) + ".bcmdl")).string(),
                  std::fstream::in | std::fstream::out | std::fstream::binary);
        if (file.is_open()) {
            char rbuf[10] = {0};
            for (const auto &offset : locale_offsets) {
                file.seekg(offset, std::fstream::beg);
                file.read(rbuf, 6);
                if (!strncmp(rbuf, locale_codes[i - 1].c_str(), 6) && !strncmp(rbuf, "USA_EN", 6))
                    return 1;
                file.seekp(offset, std::fstream::beg);
                file.write(locale_codes[i - 1].c_str(), 6);
            }
        }
        file.close();
    }
    return 0;
}

int Game::repack_cia(bool replace = false, bool verbose = false)
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
    if (replace) {
        out_cia = this->cia_path;
    } else {
        fs::path out_dir = this->cwd.parent_path().parent_path() / "out";
        fs::create_directories(out_dir);
        out_cia = out_dir / (this->name + ".cia");
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
