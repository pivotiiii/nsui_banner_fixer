#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <subprocess.hpp>
#include <vector>

#include "Game.hpp"
#include "globals.hpp"

#include "Tool.hpp"

namespace fs = std::filesystem;
namespace sp = subprocess;

const std::vector<int> locale_offsets = {0x14BC, 0x14CB};

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

Game::Game(const fs::path &cia, const Settings &set)
    : cia_path(cia),
      name(cia.stem().string()),
      cwd(fs::current_path() / "temp" / cia.stem().string()),
      set(set),
      banner_ext("bin")
{
    version = get_version();
    fs::create_directories(cwd);
}

Game::~Game()
{
    fs::remove_all(cwd.parent_path());
}

int Game::fix_banner()
{
    if (!set.quiet)
        std::cout << "--- " << cia_path.string() << "\n--- extracting cia\n";
    extract_cia();
    if (!set.quiet)
        std::cout << "--- editing banner\n";
    edit_bcmdl();
    if (!set.quiet)
        std::cout << "--- repacking cia\n";
    repack_cia();
    if (!set.quiet)
        std::cout << "--- done\n";
    return 0;
}

versionS Game::get_version()
{
    Tool::CTR ctr(cia_path, cwd, set);
    versionS version = ctr.get_cia_version();
    if (set.verbose) {
        std::cout << "Detected version: " << version.major << "." << version.minor << "." << version.micro << "\n";
    }
    return version;
}

int Game::extract_cia()
{

    // std::vector<std::string> extract_contents = {ctrtoolX.string(),
    //                                              std::string("--contents=") + (cwd / "contents").string(),
    //                                             cia_path.string()};
    // if (sp::call(extract_contents))
    //     return 1;
    Tool::CTR ctr(cia_path, cwd, set);
    int a = ctr.extract_cia_contents();

    Tool::DS ds(name, cwd, set);
    ds.split_contents();

    // std::vector<std::string> split_contents = {dstool.string(),
    //                                            "-x", "-t",
    //                                            "cxi", "-f", (cwd / "contents.0000.00000000").string(),
    //                                            "--header", (cwd / "ncch.header").string(),
    //                                            "--exh", (cwd / "exheader.bin").string(),
    //                                            "--exefs", (cwd / "exefs.bin").string(),
    //                                            "--romfs", (cwd / "romfs.bin").string()};
    // if (sp::call(split_contents))
    //     return 1;

    ds.extract_exefs();

    // std::vector<std::string> extract_exefs = {dstool.string(),
    //                                           "-x", "-t",
    //                                           "exefs", "-f", (cwd / "exefs.bin").string(),
    //                                           "--header", (cwd / "exefs.header").string(),
    //                                           "--exefs-dir", (cwd / "exefs").string()};
    // if (sp::call(extract_exefs))
    //     return 1;

    ds.extract_banner();

    // fs::create_directory(cwd / "banner");
    // if (fs::exists(cwd / "exefs" / "banner.bnr"))
    //     banner_ext = "bnr";
    //
    // std::vector<std::string> extract_banner = {dstool.string(),
    //                                           "-x", "-t",
    //                                           "banner", "-f", (cwd / "exefs" / (std::string("banner.") + banner_ext)).string(),
    //                                           "--banner-dir", (cwd / "banner").string()};
    // if (sp::call(extract_banner))
    //    return 1;

    return 0;
}

int Game::edit_bcmdl()
{
    for (int i = 1; i < 14; i++) {
        std::fstream file;
        file.open((cwd / "banner" / (std::string("banner") + std::to_string(i) + ".bcmdl")).string(),
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

int Game::repack_cia()
{
    Tool::DS ds(name, cwd, set);
    ds.rebuild_banner();

    // fs::remove(cwd / "exefs" / (std::string("banner.") + banner_ext));
    // std::vector<std::string> rebuild_banner = {dstool.string(),
    //                                            "-c", "-t",
    //                                            "banner", "-f", (cwd / "exefs" / (std::string("banner.") + banner_ext)).string(),
    //                                            "--banner-dir", (cwd / "banner").string()};
    // if (sp::call(rebuild_banner))
    //     return 1;
    ds.rebuild_exefs();
    // std::vector<std::string> rebuild_exefs = {dstool.string(),
    //                                           "-c", "-t",
    //                                           "exefs", "-f", (cwd / "exefs.bin").string(),
    //                                           "--header", (cwd / "exefs.header").string(),
    //                                           "--exefs-dir", (cwd / "exefs").string()};
    // if (sp::call(rebuild_exefs))
    //     return 1;
    ds.rebuild_cxi();
    // std::vector<std::string> rebuild_cxi = {dstool.string(),
    //                                         "-c", "-t",
    //                                         "cxi", "-f", (cwd / (name + ".cxi")).string(),
    //                                         "--header", (cwd / "ncch.header").string(),
    //                                         "--exh", (cwd / "exheader.bin").string(),
    //                                         "--exefs", (cwd / "exefs.bin").string(),
    //                                         "--romfs", (cwd / "romfs.bin").string()};
    // if (sp::call(rebuild_cxi))
    //     return 1;

    fs::path out_cia;
    if (set.replace) {
        out_cia = cia_path;
    } else {
        fs::path out_dir = cwd.parent_path().parent_path() / "out";
        fs::create_directories(out_dir);
        out_cia = out_dir / (name + ".cia");
    }

    Tool::MakeRom mr(name, out_cia, version, set);
    mr.rebuild_cia();

    // fs::path content_path_rel = fs::relative(cwd / (name + ".cxi"), fs::current_path());
    //
    // std::vector<std::string> rebuild_cia = {makerom.string(),
    //                                        "-f", "cia",
    //                                        "-o", out_cia.string(),
    //                                        "-content", content_path_rel.string() + ":0:0x00",
    //                                        "-major", std::to_string(version.major),
    //                                        "-minor", std::to_string(version.minor),
    //                                        "-micro", std::to_string(version.micro)};
    // if (sp::call(rebuild_cia))
    //    return 1;

    return 0;
}
