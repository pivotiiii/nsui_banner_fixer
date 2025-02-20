#include <filesystem>
#include <iostream>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include "Windows.h"
#undef interface
#include "tools.rc"
#endif

#ifdef GUI
#include "UI.hpp"
#else
#include "ArgsParser.hpp"
#endif

#if defined(_WIN32) && !defined(GUI)
#include <chrono>
#include <thread>
#endif

#include <pathfind.hpp>

#include "Game.hpp"
#include "Settings.hpp"

namespace fs = std::filesystem;

#ifdef _WIN32
class Codepage_Manager {
  private:
    UINT old_page;

  public:
    Codepage_Manager()
    {
        this->old_page = GetConsoleOutputCP();
        SetConsoleOutputCP(CP_UTF8);
    };
    ~Codepage_Manager()
    {
        SetConsoleOutputCP(this->old_page);
    };
};

class Resource_Manager {
  private:
    fs::path tools_dir;

  public:
    Resource_Manager(Settings &set)
    {
        const int tools_res_ids[3] = {DSTOOL, CTRTOOL, MAKEROM};
        const std::string tools_res_file[3] = {"3dstool.exe", "ctrtool.exe", "makerom.exe"};
        fs::path* tool_paths_settings[3] = {&set.dstool, &set.ctrtool, &set.makerom};

        this->tools_dir = set.cwd / "tools_nbf_temp";
        fs::create_directories(this->tools_dir);

        for (int i = 0; i < 3; i++) {
            HRSRC hResource = FindResource(nullptr, MAKEINTRESOURCE(tools_res_ids[i]), RT_RCDATA);
            if (hResource == NULL) {
                std::cerr << "ERROR: " << tools_res_file[i] << " resource can't be found!\n";
                continue;
            }
            HGLOBAL hGlobal = LoadResource(NULL, hResource);
            if (hGlobal == NULL) {
                std::cerr << "ERROR: " << tools_res_file[i] << " resource can't be loaded!\n";
                continue;
            }
            DWORD exeSize = SizeofResource(NULL, hResource);
            if (exeSize == 0) {
                std::cerr << "ERROR: " << tools_res_file[i] << " resource size is 0!\n";
                continue;
            }
            void* exeBuf = LockResource(hGlobal);
            if (exeBuf == NULL) {
                std::cerr << "ERROR: " << tools_res_file[i] << " resource can't be locked!\n";
                continue;
            }
            std::ofstream ofs((this->tools_dir / tools_res_file[i]).string().c_str(), std::ios::binary);
            if (!ofs.is_open()) {
                std::cerr << "ERROR: " << tools_res_file[i] << " can't be created!\n";
                continue;
            }
            ofs.write((char*) exeBuf, exeSize);
            ofs.close();
            *tool_paths_settings[i] = this->tools_dir / tools_res_file[i];
        }
    }
    ~Resource_Manager()
    {
        fs::remove_all(this->tools_dir);
    }
};
#endif

#if defined(_WIN32) && !defined(GUI)
bool check_requirements(std::vector<fs::path> reqs)
{
    uint8_t err_count = 0;
    for (const fs::path &path : reqs) {
        if (!fs::exists(path)) {
            std::cerr << "ERROR: " << path.filename() << " is missing!\n";
            err_count = err_count + 1;
        }
    }
    if (err_count > 0) {
        return false;
    }
    return true;
}

void pause_if_double_clicked(bool require_key_press, int sleep)
{
    DWORD procIDs[2];
    DWORD maxCount = 2;
    DWORD result = GetConsoleProcessList((LPDWORD) procIDs, maxCount);
    if (result == 1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        if (require_key_press) {
            system("pause");
        }
    }
}
#endif

#if defined(_WIN32) && defined(GUI)
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    int argc = __argc;
    char** argv = __argv;
#else
int main(int argc, char* argv[])
{
#endif

#ifdef _WIN32
    static Codepage_Manager man;
#endif

    Settings set;
    set.bin = PathFind::FindExecutable();
    set.cwd = fs::current_path();         // maybe a proper temp dir would be better
    set.out = fs::current_path() / "out"; // only used outside of GUI

#ifdef _WIN32
    static Resource_Manager res(set); // static to ensure the destructor is called after the program has finished
#endif

#ifdef GUI
    UI a(set);
    return 0;
#else
    if (!check_requirements(std::vector<fs::path> {set.dstool, set.ctrtool, set.makerom})) {
        std::cerr << "ERROR: requirements are missing!\n";
        return 1;
    }
    std::vector<fs::path> cia_paths;

    int parse_args_return = parse_args(argc, argv, cia_paths, set);
    switch (parse_args_return) {
        case 1:
            std::cerr << "ERROR: there was something wrong with the supplied arguments!\n";
            return 1;
        case 2:
            return 0;
    }

    std::vector<Fix_Banner_Result> results;
    for (const auto &path : cia_paths) {
        results.push_back(fix_cia(path, set));
    }

    for (const auto &res : results) {
        if (res.result == false) {
            std::cerr << "ERROR: There was a problem processing " << res.path.string() << "\n"
                      << res.message;
        }
    }
#ifdef _WIN32
    pause_if_double_clicked();
#endif
    return 0;
#endif
}
