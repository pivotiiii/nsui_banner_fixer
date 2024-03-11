#pragma once
#include <string>
#include <vector>

#include "Game.hpp"
#include "Settings.hpp"
#include <ctrtool/CiaProcess.h>
#include <filesystem>

int main_3dstool(int argc, char* argv[]);
namespace Tool {

class BasicTool {
  protected:
    std::vector<std::string> output_lines;
    Settings set;
    std::filesystem::path cwd;

  public:
    BasicTool(const Settings &set, const std::filesystem::path &cwd);
    std::vector<std::string> get_output_lines();
};

class CTR : public BasicTool {
  protected:
    std::filesystem::path cia;
    ctrtool::CiaProcess setup_cia_process(const std::vector<std::string> &args);

  public:
    CTR(const std::filesystem::path &cia, const std::filesystem::path &cwd, const Settings &set);
    versionS get_cia_version();
    versionS get_cia_version2();
    int extract_cia_contents();
};

class DS : public BasicTool {
  protected:
    std::string name;
    void convert_args(const std::vector<std::string> &args, int &argc, std::vector<char*> &argv);
    int run_3dstool(const std::vector<std::string> &args);

  public:
    DS(const std::string &name, const std::filesystem::path &cwd, const Settings &set);
    bool split_contents();
    bool extract_exefs();
    bool extract_banner();
    bool rebuild_banner();
    bool rebuild_exefs();
    bool rebuild_cxi();
};

class MakeRom : public BasicTool {
  protected:
    std::string name;
    versionS version;
    std::filesystem::path out_cia;

  public:
    MakeRom(const std::string &name, const std::filesystem::path &out_cia, const versionS version, const Settings &set);
    bool rebuild_cia();
};
} // namespace Tool
