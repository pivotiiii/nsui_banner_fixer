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
    Game(std::filesystem::path cia, Settings set);
    ~Game();
    int fix_banner();

  private:
    std::filesystem::path cia_path;
    std::filesystem::path cwd;
    std::string name;
    std::string banner_ext;
    versionS version;
    Settings set;

    versionS get_version();
    int extract_cia();
    int edit_bcmdl();
    int repack_cia();
};

#endif
