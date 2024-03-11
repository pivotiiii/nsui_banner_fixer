#include "Tool.hpp"

#include <exefs.h>
#include <iostream>
#include <regex>

#include <banner.h>
#include <ctrtool/CiaProcess.h>
#include <ctrtool/Settings.h>

// #include <libyaml/yaml.h>
extern "C" {
#include <makerom/lib.h>
}

extern "C" {
#include <makerom/cia_build.h>
#include <makerom/ncch_build.h>
#include <makerom/ncsd_build.h>
#include <makerom/user_settings.h>
#include <makerom/utils.h>
}
#include <ncch.h>
#include <stdcapture/stdcapture.h>

#define capture_output(x, y)                                                     \
    {                                                                            \
        std::stringstream captured;                                              \
        {                                                                        \
            std::capture::CaptureStdout cap([&](const char* buf, size_t szbuf) { \
                captured << std::string(buf, szbuf);                             \
            });                                                                  \
            x;                                                                   \
        }                                                                        \
        y.clear();                                                               \
        std::string line;                                                        \
        while (std::getline(captured, line)) {                                   \
            y.push_back(line);                                                   \
        }                                                                        \
    }

// void SetDefaults(user_settings* set);
// int CheckArgumentCombination(user_settings* set);
// int SetKeys(keys_struct* keys);

namespace fs = std::filesystem;

namespace Tool {

BasicTool::BasicTool(const Settings &set, const std::filesystem::path &cwd)
    : set(set),
      cwd(cwd)
{
}

std::vector<std::string> BasicTool::get_output_lines()
{
    return output_lines;
}

CTR::CTR(const fs::path &cia, const fs::path &cwd, const Settings &set)
    : BasicTool(set, cwd),
      cia(cia)
{
}

ctrtool::CiaProcess CTR::setup_cia_process(const std::vector<std::string> &args)
{
    std::vector<std::string> call_args = {set.bin.string()};
    call_args.insert(std::end(call_args), std::begin(args), std::end(args));
    ctrtool::CiaProcess proc;
    ctrtool::Settings ctr_set = ctrtool::SettingsInitializer(call_args);
    std::shared_ptr<tc::io::IStream> infile_stream =
        std::make_shared<tc::io::FileStream>(tc::io::FileStream(ctr_set.infile.path.get(), tc::io::FileMode::Open, tc::io::FileAccess::Read));

    proc.setInputStream(infile_stream);
    proc.setKeyBag(ctr_set.opt.keybag);
    proc.setCliOutputMode(ctr_set.opt.info);
    proc.setVerboseMode(ctr_set.opt.verbose);
    proc.setVerifyMode(ctr_set.opt.verify);
    if (ctr_set.rom.content_extract_path.isSet())
        proc.setContentExtractPath(ctr_set.rom.content_extract_path.get());
    proc.setContentIndex(ctr_set.rom.content_process_index);
    if (ctr_set.cia.certs_path.isSet())
        proc.setCertExtractPath(ctr_set.cia.certs_path.get());
    if (ctr_set.cia.tik_path.isSet())
        proc.setTikExtractPath(ctr_set.cia.tik_path.get());
    if (ctr_set.cia.tmd_path.isSet())
        proc.setTmdExtractPath(ctr_set.cia.tmd_path.get());
    if (ctr_set.cia.meta_path.isSet())
        proc.setFooterExtractPath(ctr_set.cia.meta_path.get());
    proc.setRawMode(ctr_set.opt.raw);
    proc.setPlainMode(ctr_set.opt.plain);
    proc.setShowSyscallName(ctr_set.exheader.show_syscalls_as_names);
    proc.setNcchRegionProcessOutputMode(
        ctrtool::NcchProcess::NcchRegion_Header, ctr_set.opt.info, false, tc::Optional<tc::io::Path>(), tc::Optional<tc::io::Path>());
    proc.setNcchRegionProcessOutputMode(
        ctrtool::NcchProcess::NcchRegion_ExHeader, ctr_set.opt.info, false, ctr_set.ncch.exheader_path, tc::Optional<tc::io::Path>());
    proc.setNcchRegionProcessOutputMode(
        ctrtool::NcchProcess::NcchRegion_PlainRegion, false, false, ctr_set.ncch.plainregion_path, tc::Optional<tc::io::Path>());
    proc.setNcchRegionProcessOutputMode(
        ctrtool::NcchProcess::NcchRegion_Logo, false, false, ctr_set.ncch.logo_path, tc::Optional<tc::io::Path>());
    proc.setNcchRegionProcessOutputMode(
        ctrtool::NcchProcess::NcchRegion_ExeFs, ctr_set.opt.info, ctr_set.exefs.list_fs, ctr_set.ncch.exefs_path, ctr_set.exefs.extract_path);
    proc.setNcchRegionProcessOutputMode(
        ctrtool::NcchProcess::NcchRegion_RomFs, ctr_set.opt.info, ctr_set.romfs.list_fs, ctr_set.ncch.romfs_path, ctr_set.romfs.extract_path);

    return proc;
}

versionS CTR::get_cia_version()
{
    std::vector<std::string> call_args = {"-i", cia.string()};
    ctrtool::CiaProcess proc = setup_cia_process(call_args);

    capture_output(proc.process(), output_lines);

    versionS version;
    for (auto line : output_lines) {
        if (line.starts_with("|- TitleVersion:")) {
            std::regex version_regex("(\\d+).(\\d+).(\\d+)");
            std::smatch matches;
            if (std::regex_search(line, matches, version_regex)) {
                if (matches.size() == 4) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
                    version.major = max(min(63, stoi(matches[1].str())), 0);
                    version.minor = max(min(63, stoi(matches[2].str())), 0);
                    version.micro = max(min(15, stoi(matches[3].str())), 0);
#else
                    version.major = std::max(std::min(63, stoi(matches[1].str())), 0);
                    version.minor = std::max(std::min(63, stoi(matches[2].str())), 0);
                    version.micro = std::max(std::min(15, stoi(matches[3].str())), 0);
#endif
                }
            }
            break;
        }
    }
    return version;
}

int CTR::extract_cia_contents()
{
    const std::vector<std::string> call_args = {std::string("--contents=") + (cwd / "contents").string(), cia.string()};
    ctrtool::CiaProcess proc = setup_cia_process(call_args);
    capture_output(proc.process(), output_lines);
    return 0;
}

DS::DS(const std::string &name, const fs::path &cwd, const Settings &set)
    : BasicTool(set, cwd),
      name(name)
{
}

void DS::convert_args(const std::vector<std::string> &args, int &argc, std::vector<char*> &argv)
{
    std::vector<std::string> call_args = {set.bin.string()};
    call_args.insert(std::end(call_args), std::begin(args), std::end(args));

    auto convert = [](const std::string &s) -> char* {
        char* pc = new char[s.size() + 1];
        std::strcpy(pc, s.c_str());
        return pc;
    };

    std::transform(call_args.begin(), call_args.end(), std::back_inserter(argv), convert);

    // argv.reserve(args.size() + 1);
    // argv.push_back(set.bin.string().c_str());
    // for (int i = 0; i < args.size(); ++i) {
    //     argv.push_back(args[i].c_str());
    // }
    argc = args.size() + 1;
}

int DS::run_3dstool(const std::vector<std::string> &args)
{
    int argc_like = 0;
    std::vector<char*> argv_like = {};
    convert_args(args, argc_like, argv_like);
    int a = main_3dstool(argc_like, &argv_like[0]);
    for (size_t i = 0; i < argv_like.size(); i++)
        delete[] argv_like[i];
    return a;
}

bool DS::split_contents()
{
    std::filesystem::path contents = cwd / "contents.0000.00000000";
    std::cout << "ds.split_contents cwd: " << cwd.string() << "\n";
    if (std::filesystem::exists(contents)) {
        CNcch ncch;
        ncch.SetFileName(contents.string());
        ncch.SetVerbose(set.verbose);
        ncch.SetHeaderFileName((cwd / "ncch.header").string());
        ncch.SetEncryptMode(3);
        ncch.SetDev(false);
        ncch.SetExtendedHeaderFileName((cwd / "exheader.bin").string());
        ncch.SetLogoRegionFileName("");
        ncch.SetPlainRegionFileName("");
        ncch.SetExeFsFileName((cwd / "exefs.bin").string());
        ncch.SetRomFsFileName((cwd / "romfs.bin").string());
        bool bResult = ncch.ExtractFile();
        return bResult;
    } else {
        std::cerr << "ERROR: can't find " << contents.string() << "\n";
        return false;
    }
}

bool DS::extract_exefs()
{
    std::filesystem::path exefs = cwd / "exefs.bin";
    if (std::filesystem::exists(exefs)) {
        CExeFs exeFs;
        exeFs.SetFileName((cwd / "exefs.bin").string());
        exeFs.SetVerbose(set.verbose);
        exeFs.SetHeaderFileName((cwd / "exefs.header").string());
        exeFs.SetExeFsDirName((cwd / "exefs").string());
        exeFs.SetUncompress(false);
        return exeFs.ExtractFile();
    } else {
        std::cerr << "ERROR: can't find " << exefs.string() << "\n";
        return false;
    }
}

bool DS::extract_banner()
{
    fs::create_directory(cwd / "banner");
    std::string banner_ext = "bin";
    if (fs::exists(cwd / "exefs" / "banner.bnr"))
        banner_ext = "bnr";

    CBanner banner;
    banner.SetFileName((cwd / "exefs" / (std::string("banner.") + banner_ext)).string());
    banner.SetVerbose(set.verbose);
    banner.SetBannerDirName((cwd / "banner").string());
    return banner.ExtractFile();

    // const std::vector<std::string> call_args = {"-x", "-t", "banner", "-f", (cwd / "exefs" / (std::string("banner.") + banner_ext)).string(), "--banner-dir", (cwd / "banner").string()};
    // run_3dstool(call_args);

    return 1;
}

bool DS::rebuild_banner()
{
    std::string banner_ext = "bin";
    if (fs::exists(cwd / "exefs" / "banner.bnr"))
        banner_ext = "bnr";
    fs::remove(cwd / "exefs" / (std::string("banner.") + banner_ext));

    CBanner banner;
    banner.SetFileName((cwd / "exefs" / (std::string("banner.") + banner_ext)).string());
    banner.SetVerbose(set.verbose);
    banner.SetBannerDirName((cwd / "banner").string());
    return banner.CreateFile();
}

bool DS::rebuild_exefs()
{
    CExeFs exeFs;
    exeFs.SetFileName((cwd / "exefs.bin").string());
    exeFs.SetVerbose(set.verbose);
    exeFs.SetHeaderFileName((cwd / "exefs.header").string());
    exeFs.SetExeFsDirName((cwd / "exefs").string());
    exeFs.SetCompress(false);
    return exeFs.CreateFile();
}

bool DS::rebuild_cxi()
{
    CNcch ncch;
    ncch.SetFileName((cwd / (name + ".cxi")).string());
    ncch.SetVerbose(set.verbose);
    ncch.SetHeaderFileName((cwd / "ncch.header").string());
    ncch.SetEncryptMode(3);
    ncch.SetRemoveExtKey(true);
    ncch.SetDev(false);
    ncch.SetExtendedHeaderFileName((cwd / "exheader.bin").string());
    ncch.SetLogoRegionFileName("");
    ncch.SetPlainRegionFileName("");
    ncch.SetExeFsFileName((cwd / "exefs.bin").string());
    ncch.SetRomFsFileName((cwd / "romfs.bin").string());
    return ncch.CreateFile();
}

MakeRom::MakeRom(const std::string &name, const std::filesystem::path &out_cia, const versionS version, const Settings &set)
    : BasicTool(set, set.cwd),
      name(name),
      out_cia(out_cia),
      version(version)
{
}

bool MakeRom::rebuild_cia()
{
    std::string cxi = (cwd / "temp" / name / (name + ".cxi")).string();
    user_settings mr_set;

    init_UserSettings(&mr_set);
    initRand();
    InitKeys(&mr_set.common.keys);
    SetDefaults(&mr_set);
    mr_set.common.outFormat = CIA;
    mr_set.common.outFileName = const_cast<char*>(out_cia.c_str());
    mr_set.common.outFileName_mallocd = false;
    mr_set.common.verbose = set.verbose;
    mr_set.cia.useNormTitleVer = true;
    mr_set.cia.titleVersion[VER_MAJOR] = version.major;
    mr_set.cia.titleVersion[VER_MINOR] = version.minor;
    mr_set.cia.titleVersion[VER_MICRO] = version.micro;
    mr_set.common.contentPath = (char**) calloc(CIA_MAX_CONTENT, sizeof(char*));
    mr_set.common.contentPath[0] = cxi.data();
    mr_set.common.contentSize[0] = GetFileSize64(cxi.data());
    mr_set.cia.contentId[0] = 0;
    SetKeys(&mr_set.common.keys);

    FILE* ncch0 = fopen(mr_set.common.contentPath[0], "rb");
    ncch_hdr hdr;

    ReadNcchHdr(&hdr, ncch0);
    u64 fileSize = GetFileSize64(mr_set.common.contentPath[0]);
    mr_set.common.workingFile.size = fileSize;
    mr_set.common.workingFile.buffer = (u8*) malloc(fileSize);
    ReadFile64(mr_set.common.workingFile.buffer, mr_set.common.workingFile.size, 0, ncch0);
    fclose(ncch0);

    build_CIA(&mr_set);
    free(mr_set.common.contentPath);
    return true;
}
} // namespace Tool
