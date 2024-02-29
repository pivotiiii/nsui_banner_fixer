#ifndef GAME_H
#define GAME_H

#include <filesystem>

typedef struct versionS {
    int major = 0;
    int minor = 0;
    int micro = 0;
} versionS;

class Game {
  public:
    Game(std::filesystem::path cia);
    ~Game();
    int fix_banner(bool replace, bool verbose);

  private:
    std::filesystem::path cia_path;
    std::filesystem::path cwd;
    std::string name;
    std::string banner_ext;
    versionS version;

    versionS get_version();
    int extract_cia(bool verbose);
    int edit_bcmdl(bool verbose);
    int repack_cia(bool replace, bool verbose);
};

#endif
