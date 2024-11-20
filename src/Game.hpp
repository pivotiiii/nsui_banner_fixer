#ifndef GAME_H
#define GAME_H

#include "Settings.hpp"
#include <filesystem>

typedef struct versionS {
    int major = 0;
    int minor = 0;
    int micro = 0;
} versionS;

typedef struct Fix_Banner_Result {
    std::filesystem::path path;
    bool result;
    std::string message;

    Fix_Banner_Result() {};
    Fix_Banner_Result(std::filesystem::path _path, bool _result, std::string _message = "")
        : path(_path),
          result(_result),
          message(_message) {
          };
} Fix_Banner_Result;

typedef struct Proc_Result {
    bool result;
    std::string message;

    Proc_Result() {};
    Proc_Result(bool _result, std::string _message = "")
        : result(_result),
          message(_message) {
          };
} Proc_Result;

Fix_Banner_Result fix_cia(const std::filesystem::path &path, const Settings &set);

class Game {
  public:
    Game(const std::filesystem::path &cia, const Settings &set);
    ~Game();
    Fix_Banner_Result fix_banner();

  private:
    Settings set;
    std::filesystem::path cia_path;
    std::filesystem::path cia_path_work;
    std::filesystem::path cwd;
    std::string name;
    std::string name_work;
    std::string banner_ext;
    versionS version;

    versionS get_version();
    Proc_Result extract_cia();
    Proc_Result edit_bcmdl();
    Proc_Result repack_cia();
};

#endif
