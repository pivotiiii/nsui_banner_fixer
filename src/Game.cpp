#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <subprocess.hpp>
#include <vector>

#include "Game.hpp"
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

bool Game::fix_banner()
{
    if (!set.quiet) {
        std::cout << "--- " << cia_path.string() << "\n--- extracting cia\n";
    }
    if (!extract_cia()) {
        std::cerr << "ERROR: Failed to extract CIA\n";
        return false;
    }

    if (!set.quiet) {
        std::cout << "--- editing banner\n";
    }
    if (!edit_bcmdl()) {
        std::cerr << "ERROR: Failed to edit banner files\n";
        return false;
    }

    if (!set.quiet) {
        std::cout << "--- repacking cia\n";
    }
    if (!repack_cia()) {
        std::cerr << "ERROR: Failed to repack CIA\n";
        return false;
    }

    if (!set.quiet) {
        std::cout << "--- done\n";
    }
    return true;
}

versionS Game::get_version()
{
    Tool::CTR ctr(cia_path, cwd, set);
    versionS version = ctr.get_cia_version();
    if (set.verbose) {
        std::cout << name << "\n";
        std::cout << "Detected version: " << version.major << "." << version.minor << "." << version.micro << "\n";
    }
    return version;
}

bool Game::extract_cia()
{
    Tool::CTR ctr(cia_path, cwd, set);
    if (!ctr.extract_cia_contents()) {
        std::cerr << "ERROR: Failed to extract contents from .CIA\n";
        return false;
    }

    Tool::DS ds(name, cwd, set);
    if (!ds.split_contents()) {
        std::cerr << "ERROR: Failed to split contents\n";
        return false;
    }

    if (!ds.extract_exefs()) {
        std::cerr << "ERROR: Failed to extract exefs from contents\n";
        return false;
    }

    if (!ds.extract_banner()) {
        std::cerr << "ERROR: Failed to extract banner from exefs\n";
        return false;
    }

    return true;
}

bool Game::edit_bcmdl()
{
    for (int i = 1; i < 14; i++) {
        std::fstream file;
        file.open((cwd / "banner" / (std::string("banner") + std::to_string(i) + ".bcmdl")).string(),
                  std::fstream::in | std::fstream::out | std::fstream::binary);
        if (file.is_open()) {
            if (set.verbose)
                std::cout << "Editing banner" << i << ".bcmdl\n";
            char rbuf[10] = {0};
            for (const auto &offset : locale_offsets) {
                file.seekg(offset, std::fstream::beg);
                file.read(rbuf, 6);
                if (set.verbose) {
                    std::cout << "Locale code at 0x" << std::hex << offset << std::dec << ": " << rbuf;
                }
                if (strncmp(rbuf, "USA_EN", 6) != 0 && strncmp(rbuf, locale_codes[i - 1].c_str(), 6) != 0) {
                    std::cerr << "ERROR: banner" << i << ".bcmdl no locale code at offset " << std::hex << offset << std::dec << "\n";
                    return false;
                }

                file.seekp(offset, std::fstream::beg);
                file.write(locale_codes[i - 1].c_str(), 6);
                if (set.verbose) {
                    file.seekg(offset, std::fstream::beg);
                    file.read(rbuf, 6);
                    std::cout << "--> " << rbuf << "\n";
                }
            }
        } else {
            std::cerr << "ERROR: Failed to open banner" << i << ".bcmdl\n";
            return false;
        }
        file.close();
    }
    return true;
}

bool Game::repack_cia()
{
    Tool::DS ds(name, cwd, set);

    if (!ds.rebuild_banner()) {
        std::cerr << "ERROR: Failed to rebuild banner\n";
        return false;
    }

    if (!ds.rebuild_exefs()) {
        std::cerr << "ERROR: Failed to rebuild exefs\n";
        return false;
    }

    if (!ds.rebuild_cxi()) {
        std::cerr << "ERROR: Failed to rebuild cxi\n";
        return false;
    }

    fs::path out_cia;
    if (set.replace) {
        out_cia = cia_path;
    } else {
        fs::path out_dir = cwd.parent_path().parent_path() / "out";
        fs::create_directories(out_dir);
        out_cia = out_dir / (name + ".cia");
    }

    Tool::MakeRom mr(name, out_cia, version, set);

    if (!mr.rebuild_cia()) {
        std::cerr << "ERROR: Failed to rebuild CIA\n";
        return false;
    }

    return true;
}
