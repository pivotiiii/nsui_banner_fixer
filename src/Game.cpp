#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "Game.hpp"

#if defined(_WIN32)

#include <boost/process.hpp>
#include <boost/process/windows.hpp>
#include <regex>

namespace bp = boost::process;

#define run_process(exec, arg_list, verbose_offset, ret_code, err_msg)        \
    {                                                                         \
        if (set.verbose) {                                                    \
            arg_list.insert(arg_list.begin() + verbose_offset, "-v");         \
        }                                                                     \
        bp::ipstream output_stream;                                           \
        std::string line;                                                     \
        int retval = bp::system(bp::exe = exec,                               \
                                bp::args = arg_list,                          \
                                bp::std_out > output_stream,                  \
                                ::boost::process::windows::hide);             \
                                                                              \
        if (retval != ret_code) {                                             \
            while (std::getline(output_stream, line)) {                       \
                std::cerr << line << "\n";                                    \
            }                                                                 \
            std::string err = "";                                             \
            err = std::string(err_msg) + " (" + std::to_string(retval) + ")"; \
            return Proc_Result(false, err_msg);                               \
        }                                                                     \
                                                                              \
        if (set.verbose) {                                                    \
            while (std::getline(output_stream, line)) {                       \
                std::cout << line << "\n";                                    \
            }                                                                 \
        }                                                                     \
    }
#elif defined(__linux__)

#include "Tool.hpp"

#endif

namespace fs = std::filesystem;

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

Fix_Banner_Result fix_cia(const fs::path &path, const Settings &set)
{
    Fix_Banner_Result res;
    try {
        res = Game(path, set).fix_banner();
    } catch (const std::system_error &e) { // this happens if e.g. the console is set to russian codepage and the cia path contains an accent somewhere
        res.message = e.what();
        res.message.append("\nSometimes this happens if your OS is set to a language other than English and the cia path contains accents or other special characters "
                           "(Both the full path to the folder the .cia file is in as well as the file itself). "
                           "If this is the case, please try renaming and moving the .cia file to a location without these characters, e.g. \"C:/\" and running again there.");
        res.path = fs::relative(fs::current_path() / "path" / "with" / "problems");
        res.result = false;
    }

    return res;
}

Game::Game(const fs::path &cia, const Settings &set)
    : cia_path(cia),
      name(cia.stem().string()),
      cwd(fs::current_path() / "temp" / cia.stem().string()),
      set(set),
      banner_ext("bin")
{
    this->version = get_version();
    fs::create_directories(cwd);
}

Game::~Game()
{
    fs::remove_all(this->cwd.parent_path());
}

versionS Game::get_version()
{
#if defined(_WIN32)

    versionS version;
    bp::ipstream output_stream;
    std::string line;

    bp::system(bp::exe = set.ctrtool.string(),
               bp::args = {"-i",
                           this->cia_path.string()},
               bp::std_out > output_stream,
               ::boost::process::windows::hide);

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

#elif defined(__linux__)

    Tool::CTR ctr(cia_path, cwd, set);
    versionS version = ctr.get_cia_version();

#endif
    if (set.verbose) {
        std::cout << name << "\n";
        std::cout << "Detected version: " << version.major << "." << version.minor << "." << version.micro << "\n";
    }
    return version;
}

Proc_Result Game::extract_cia()
{
#if defined(_WIN32)

    std::vector<std::string> extract_contents = {std::string("--contents=") + (this->cwd / "contents").string(),
                                                 this->cia_path.string()};
    run_process(set.ctrtool.string(), extract_contents, 1, 0, "Failed to extract contents from .CIA");

    std::vector<std::string> split_contents = {"-x", "-t",
                                               "cxi", "-f", (this->cwd / "contents.0000.00000000").string(),
                                               "--header", (this->cwd / "ncch.header").string(),
                                               "--exh", (this->cwd / "exheader.bin").string(),
                                               "--exefs", (this->cwd / "exefs.bin").string(),
                                               "--romfs", (this->cwd / "romfs.bin").string()};
    run_process(set.dstool.string(), split_contents, 1, 0, "Failed to split contents");

    std::vector<std::string> extract_exefs = {"-x", "-t",
                                              "exefs", "-f", (this->cwd / "exefs.bin").string(),
                                              "--header", (this->cwd / "exefs.header").string(),
                                              "--exefs-dir", (this->cwd / "exefs").string()};
    run_process(set.dstool.string(), extract_exefs, 1, 0, "Failed to extract exefs from contents");

    fs::create_directory(this->cwd / "banner");
    if (fs::exists(this->cwd / "exefs" / "banner.bnr")) {
        this->banner_ext = "bnr";
    }

    std::vector<std::string> extract_banner = {"-x", "-t",
                                               "banner", "-f", (this->cwd / "exefs" / (std::string("banner.") + this->banner_ext)).string(),
                                               "--banner-dir", (this->cwd / "banner").string()};
    run_process(set.dstool.string(), extract_banner, 1, 0, "Failed to extract banner from exefs");

    return Proc_Result(true);

#elif defined(__linux__)

    Tool::CTR ctr(cia_path, cwd, set);
    if (!ctr.extract_cia_contents()) {
        return Proc_Result(false, "Failed to extract contents from .CIA");
    }

    Tool::DS ds(name, cwd, set);
    if (!ds.split_contents()) {
        return Proc_Result(false, "Failed to split contents");
    }

    if (!ds.extract_exefs()) {
        return Proc_Result(false, "Failed to extract exefs from contents");
    }

    if (!ds.extract_banner()) {
        return Proc_Result(false, "Failed to extract banner from exefs");
    }

    return Proc_Result(true);

#endif
}

Fix_Banner_Result Game::fix_banner()
{
    Proc_Result pr;

    if (!set.quiet) {
        std::cout << "--- " << this->cia_path.string() << "\n--- extracting cia\n";
    }
    pr = this->extract_cia();
    if (!pr.result) {
        pr.message = "ERROR: Failed to extract CIA\nERROR: " + pr.message + "\n";
        std::cerr << pr.message;
        return Fix_Banner_Result(this->cia_path, false, pr.message);
    }

    if (!set.quiet) {
        std::cout << "--- editing banner\n";
    }
    pr = this->edit_bcmdl();
    if (!pr.result) {
        pr.message = "ERROR: Failed to edit banner files\nERROR: " + pr.message + "\n";
        std::cerr << pr.message;
        return Fix_Banner_Result(this->cia_path, false, pr.message);
    }

    if (!set.quiet) {
        std::cout << "--- repacking cia\n";
    }
    pr = this->repack_cia();
    if (!pr.result) {
        pr.message = "ERROR: Failed to repack CIA\nERROR: " + pr.message + "\n";
        std::cerr << pr.message;
        return Fix_Banner_Result(this->cia_path, false, pr.message);
    }

    if (!set.quiet) {
        std::cout << "--- done\n";
    }
    return Fix_Banner_Result(this->cia_path, true);
    ;
}

Proc_Result Game::edit_bcmdl()
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
                    std::stringstream s;
                    s << "banner" << i << ".bcmdl no locale code at offset " << std::hex << offset << std::dec
                      << ", data is \"" << std::string_view {rbuf, 6} << "\"";
                    return Proc_Result(false, s.str());
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
            std::string err_msg = "Failed to open banner" + std::to_string(i) + ".bcmdl\nPlease make sure the .cia file has been created with NSUI v28 and the 3D GBA banner.";
            return Proc_Result(false, err_msg);
        }
        file.close();
    }
    return Proc_Result(true);
}

Proc_Result Game::repack_cia()
{
#if defined(_WIN32)
    fs::remove(this->cwd / "exefs" / (std::string("banner.") + this->banner_ext));
    std::vector<std::string> rebuild_banner = {"-c", "-t",
                                               "banner", "-f", (this->cwd / "exefs" / (std::string("banner.") + this->banner_ext)).string(),
                                               "--banner-dir", (this->cwd / "banner").string()};
    run_process(set.dstool.string(), rebuild_banner, 1, 0, "Failed to rebuild banner");

    std::vector<std::string> rebuild_exefs = {"-c", "-t",
                                              "exefs", "-f", (this->cwd / "exefs.bin").string(),
                                              "--header", (this->cwd / "exefs.header").string(),
                                              "--exefs-dir", (this->cwd / "exefs").string()};
    run_process(set.dstool.string(), rebuild_exefs, 1, 0, "Failed to rebuild exefs");

    std::vector<std::string> rebuild_cxi = {"-c", "-t",
                                            "cxi", "-f", (this->cwd / (this->name + ".cxi")).string(),
                                            "--header", (this->cwd / "ncch.header").string(),
                                            "--exh", (this->cwd / "exheader.bin").string(),
                                            "--exefs", (this->cwd / "exefs.bin").string(),
                                            "--romfs", (this->cwd / "romfs.bin").string()};
    run_process(set.dstool.string(), rebuild_cxi, 1, 0, "Failed to rebuild cxi");

    fs::path out_cia;
    if (set.replace) {
        out_cia = this->cia_path;
    } else {
        fs::create_directories(this->set.out);
        out_cia = this->set.out / (this->name + ".cia");
    }
    fs::path content_path_rel = fs::relative(this->cwd / (this->name + ".cxi"), fs::current_path()); // may need checking if file on different drive

    std::vector<std::string> rebuild_cia = {"-f", "cia",
                                            "-o", out_cia.string(),
                                            "-content", content_path_rel.string() + ":0:0x00",
                                            "-major", std::to_string(this->version.major),
                                            "-minor", std::to_string(this->version.minor),
                                            "-micro", std::to_string(this->version.micro)};
    run_process(set.makerom.string(), rebuild_cia, 0, 0, "Failed to rebuild CIA");

    return Proc_Result(true);

#elif defined(__linux__)

    Tool::DS ds(name, cwd, set);

    if (!ds.rebuild_banner()) {
        return Proc_Result(false, "Failed to rebuild banner");
    }

    if (!ds.rebuild_exefs()) {
        return Proc_Result(false, "Failed to rebuild exefs");
    }

    if (!ds.rebuild_cxi()) {
        return Proc_Result(false, "Failed to rebuild cxi");
    }

    fs::path out_cia;
    if (set.replace) {
        out_cia = this->cia_path;
    } else {
        fs::create_directories(this->set.out);
        out_cia = this->set.out / (this->name + ".cia");
    }

    Tool::MakeRom mr(name, out_cia, version, set);

    if (!mr.rebuild_cia()) {
        return Proc_Result(false, "Failed to rebuild CIA");
    }

    return Proc_Result(true);

#endif
}
