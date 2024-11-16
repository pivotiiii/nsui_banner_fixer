#pragma once

#include <filesystem>
#include <mutex>
#include <vector>

#include <saucer/smartview.hpp>

#include "Game.hpp"
#include "Settings.hpp"

class UI {
  private:
    Settings set;
    std::vector<Fix_Banner_Result> cia_files;
    std::string license;

    std::shared_ptr<saucer::application> app;
    saucer::smartview<saucer::serializers::glaze::serializer> smartview;

    std::string add_cias();
    void add_path(const std::filesystem::path &path);
    void remove_path(const std::filesystem::path &path);
    void remove_path(const unsigned int &index);
    std::vector<std::filesystem::path> split_into_paths(const std::string &str, char delimiter);
    std::string build_path_json(bool show_results);
    std::string remove_cia(const int &req);
    std::string fix_banners();
    void set_replace_files(const bool &replace);
    std::string check_requirements();
    std::string get_program_info();
    bool select_save_location();
    void expose_functions();

  public:
    UI(Settings &set);
    ~UI();
};
