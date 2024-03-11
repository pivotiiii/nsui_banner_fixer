#ifndef GAME_H
#define GAME_H

#include "Settings.hpp"
#include <filesystem>

typedef struct versionS {
    int major = 0;
    int minor = 0;
    int micro = 0;
} versionS;

class Game {
  public:
    Game(const std::filesystem::path &cia, const Settings &set);
    ~Game();
    int fix_banner();

  private:
    const std::filesystem::path cia_path;
    const std::filesystem::path cwd;
    const std::string name;
    std::string banner_ext;
    versionS version;
    const Settings set;

    versionS get_version();
    int extract_cia();
    int edit_bcmdl();
    int repack_cia();
};

#endif
