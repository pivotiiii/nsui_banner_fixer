#include "Ctrtool_cia_proc.hpp"

#include <iostream>
#include <sstream>

#include <ctrtool/Settings.h>
#include <stdcapture/stdcapture.h>

Ctrtool_cia_proc::Ctrtool_cia_proc(std::vector<std::string> &args)
{
    // std::vector<std::string> args = {"/mnt/d/Documents/VS Code/nsui_banner_fixer/cpp/build/nsui_banner_fixer", "-i", "/mnt/d/Documents/VS Code/nsui_banner_fixer/cpp/src/Castlevania Aria of Sorrow.cia"};
    ctrtool::Settings set = ctrtool::SettingsInitializer(args);
    std::shared_ptr<tc::io::IStream> infile_stream = std::make_shared<tc::io::FileStream>(tc::io::FileStream(set.infile.path.get(), tc::io::FileMode::Open, tc::io::FileAccess::Read));
    this->proc.setInputStream(infile_stream);
    this->proc.setKeyBag(set.opt.keybag);
    this->proc.setCliOutputMode(set.opt.info);
    this->proc.setVerboseMode(set.opt.verbose);
    this->proc.setVerifyMode(set.opt.verify);
    if (set.rom.content_extract_path.isSet())
        this->proc.setContentExtractPath(set.rom.content_extract_path.get());
    this->proc.setContentIndex(set.rom.content_process_index);
    if (set.cia.certs_path.isSet())
        this->proc.setCertExtractPath(set.cia.certs_path.get());
    if (set.cia.tik_path.isSet())
        this->proc.setTikExtractPath(set.cia.tik_path.get());
    if (set.cia.tmd_path.isSet())
        this->proc.setTmdExtractPath(set.cia.tmd_path.get());
    if (set.cia.meta_path.isSet())
        this->proc.setFooterExtractPath(set.cia.meta_path.get());
    this->proc.setRawMode(set.opt.raw);
    this->proc.setPlainMode(set.opt.plain);
    this->proc.setShowSyscallName(set.exheader.show_syscalls_as_names);
    this->proc.setNcchRegionProcessOutputMode(ctrtool::NcchProcess::NcchRegion_Header, set.opt.info, false, tc::Optional<tc::io::Path>(), tc::Optional<tc::io::Path>());
    this->proc.setNcchRegionProcessOutputMode(ctrtool::NcchProcess::NcchRegion_ExHeader, set.opt.info, false, set.ncch.exheader_path, tc::Optional<tc::io::Path>());
    this->proc.setNcchRegionProcessOutputMode(ctrtool::NcchProcess::NcchRegion_PlainRegion, false, false, set.ncch.plainregion_path, tc::Optional<tc::io::Path>());
    this->proc.setNcchRegionProcessOutputMode(ctrtool::NcchProcess::NcchRegion_Logo, false, false, set.ncch.logo_path, tc::Optional<tc::io::Path>());
    this->proc.setNcchRegionProcessOutputMode(ctrtool::NcchProcess::NcchRegion_ExeFs, set.opt.info, set.exefs.list_fs, set.ncch.exefs_path, set.exefs.extract_path);
    this->proc.setNcchRegionProcessOutputMode(ctrtool::NcchProcess::NcchRegion_RomFs, set.opt.info, set.romfs.list_fs, set.ncch.romfs_path, set.romfs.extract_path);
}

void Ctrtool_cia_proc::run(bool silent = true)
{
    std::stringstream captured;
    {
        std::capture::CaptureStdout cap([&](const char* buf, size_t szbuf) {
            captured << std::string(buf, szbuf);
        });
        this->proc.process();
    }
    this->output_lines.clear();
    std::string line;
    while (std::getline(captured, line)) {
        this->output_lines.push_back(line);
    }
}

std::vector<std::string> Ctrtool_cia_proc::get_output_lines()
{
    return this->output_lines;
}
