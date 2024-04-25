#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <vector>

#include <subprocess.hpp>

#include "Game.hpp"

#define run_process(name, verbose_offset, ret_code, err_msg)                         \
    {                                                                                \
        if (set.verbose) {                                                           \
            name.insert(name.begin() + verbose_offset, "-v");                        \
        }                                                                            \
                                                                                     \
        sp::Popen process = sp::Popen(name, sp::output {sp::PIPE});                  \
        auto obuf = process.communicate().first;                                     \
                                                                                     \
        if (process.retcode() != ret_code) {                                         \
            std::cerr << obuf.buf.data() << "\n";                                    \
            std::cerr << "ERROR: " << err_msg << " (" << process.retcode() << ")\n"; \
            return false;                                                            \
        }                                                                            \
                                                                                     \
        if (set.verbose) {                                                           \
            std::cerr << obuf.buf.data() << "\n";                                    \
        }                                                                            \
    }

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
    this->version = get_version();
    fs::create_directories(this->cwd);
}

Game::~Game()
{
    fs::remove_all(this->cwd.parent_path());
}

bool Game::fix_banner()
{
    if (!set.quiet) {
        std::cout << "--- " << this->cia_path.string() << "\n--- extracting cia\n";
    }
    if (!this->extract_cia()) {
        std::cerr << "ERROR: Failed to extract CIA\n";
        return false;
    }

    if (!set.quiet) {
        std::cout << "--- editing banner\n";
    }
    if (!this->edit_bcmdl()) {
        std::cerr << "ERROR: Failed to edit banner files\n";
        return false;
    }

    if (!set.quiet) {
        std::cout << "--- repacking cia\n";
    }
    if (!this->repack_cia()) {
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
    versionS version;

    auto output_buffer = sp::check_output({set.ctrtool.string(), "-i", this->cia_path.string()});
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
    if (set.verbose) {
        std::cout << name << "\n";
        std::cout << "Detected version: " << version.major << "." << version.minor << "." << version.micro << "\n";
    }
    return version;
}

bool Game::extract_cia()
{
    std::vector<std::string> extract_contents = {set.ctrtool.string(),
                                                 std::string("--contents=") + (this->cwd / "contents").string(),
                                                 this->cia_path.string()};
    run_process(extract_contents, 1, 0, "Failed to extract contents from .CIA");

    std::vector<std::string> split_contents = {set.dstool.string(),
                                               "-x", "-t",
                                               "cxi", "-f", (this->cwd / "contents.0000.00000000").string(),
                                               "--header", (this->cwd / "ncch.header").string(),
                                               "--exh", (this->cwd / "exheader.bin").string(),
                                               "--exefs", (this->cwd / "exefs.bin").string(),
                                               "--romfs", (this->cwd / "romfs.bin").string()};
    run_process(split_contents, 2, 0, "Failed to split contents");

    std::vector<std::string> extract_exefs = {set.dstool.string(),
                                              "-x", "-t",
                                              "exefs", "-f", (this->cwd / "exefs.bin").string(),
                                              "--header", (this->cwd / "exefs.header").string(),
                                              "--exefs-dir", (this->cwd / "exefs").string()};
    run_process(extract_exefs, 2, 0, "Failed to extract exefs from contents");

    fs::create_directory(this->cwd / "banner");
    if (fs::exists(this->cwd / "exefs" / "banner.bnr")) {
        this->banner_ext = "bnr";
    }

    std::vector<std::string> extract_banner = {set.dstool.string(),
                                               "-x", "-t",
                                               "banner", "-f", (this->cwd / "exefs" / (std::string("banner.") + this->banner_ext)).string(),
                                               "--banner-dir", (this->cwd / "banner").string()};
    run_process(extract_banner, 2, 0, "Failed to extract banner from exefs");

    return true;
}

bool Game::edit_bcmdl()
{
    for (int i = 1; i < 14; i++) {
        std::fstream file;
        file.open((this->cwd / "banner" / (std::string("banner") + std::to_string(i) + ".bcmdl")).string(),
                  std::fstream::in | std::fstream::out | std::fstream::binary);
        if (file.is_open()) {
            if (set.verbose) {
                std::cout << "Editing banner" << i << ".bcmdl\n";
            }
            char rbuf[10] = {0};
            for (const auto &offset : locale_offsets) {
                file.seekg(offset, std::fstream::beg);
                file.read(rbuf, 6);
                if (set.verbose) {
                    std::cout << "Locale code at 0x" << std::hex << offset << std::dec << ": " << rbuf;
                }
                if (strncmp(rbuf, "USA_EN", 6) != 0 && strncmp(rbuf, locale_codes[i - 1].c_str(), 6) != 0) {
                    std::cerr << "ERROR: banner" << i << ".bcmdl no locale code at offset " << std::hex << offset << std::dec
                              << ", data is \"" << std::string_view {rbuf, 6} << "\"\n";
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
            std::cerr << "Was " << this->name << ".cia created with NSUI v28 using the 3D GBA banner?\n";
            return false;
        }
        file.close();
    }
    return true;
}

bool Game::repack_cia()
{
    fs::remove(this->cwd / "exefs" / (std::string("banner.") + this->banner_ext));
    std::vector<std::string> rebuild_banner = {set.dstool.string(),
                                               "-c", "-t",
                                               "banner", "-f", (this->cwd / "exefs" / (std::string("banner.") + this->banner_ext)).string(),
                                               "--banner-dir", (this->cwd / "banner").string()};
    run_process(rebuild_banner, 2, 0, "Failed to rebuild banner");

    std::vector<std::string> rebuild_exefs = {set.dstool.string(),
                                              "-c", "-t",
                                              "exefs", "-f", (this->cwd / "exefs.bin").string(),
                                              "--header", (this->cwd / "exefs.header").string(),
                                              "--exefs-dir", (this->cwd / "exefs").string()};
    run_process(rebuild_exefs, 2, 0, "Failed to rebuild exefs");

    std::vector<std::string> rebuild_cxi = {set.dstool.string(),
                                            "-c", "-t",
                                            "cxi", "-f", (this->cwd / (this->name + ".cxi")).string(),
                                            "--header", (this->cwd / "ncch.header").string(),
                                            "--exh", (this->cwd / "exheader.bin").string(),
                                            "--exefs", (this->cwd / "exefs.bin").string(),
                                            "--romfs", (this->cwd / "romfs.bin").string()};
    run_process(rebuild_cxi, 2, 0, "Failed to rebuild cxi");

    fs::path out_cia;
    if (set.replace) {
        out_cia = this->cia_path;
    } else {
        fs::path out_dir = this->cwd.parent_path().parent_path() / "out";
        fs::create_directories(out_dir);
        out_cia = out_dir / (this->name + ".cia");
    }
    fs::path content_path_rel = fs::relative(this->cwd / (this->name + ".cxi"), fs::current_path()); // may need checking if file on different drive

    std::vector<std::string> rebuild_cia = {set.makerom.string(),
                                            "-f", "cia",
                                            "-o", out_cia.string(),
                                            "-content", content_path_rel.string() + ":0:0x00",
                                            "-major", std::to_string(this->version.major),
                                            "-minor", std::to_string(this->version.minor),
                                            "-micro", std::to_string(this->version.micro)};
    run_process(rebuild_cia, 1, 0, "Failed to rebuild CIA");

    return true;
}
