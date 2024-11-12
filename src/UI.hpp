#pragma once

#include <filesystem>
#include <mutex>
#include <vector>

#include <saucer/smartview.hpp>

#include "Settings.hpp"
#include "nsui_banner_fixer.hpp"

class UI {
  private:
    Settings set;
    std::vector<Cia_File> cia_files;

    std::shared_ptr<saucer::application> app;
    saucer::smartview<saucer::serializers::glaze::serializer> smartview;

    void quit();

    /*std::string build_final_html(std::string &html);
    void load_nsui_page();
    void add_path(const std::filesystem::path &path);
    void remove_path(const std::filesystem::path &path);
    void remove_path(const unsigned int &index);
    std::vector<std::filesystem::path> split_into_paths(const std::string &str, char delimiter);
    std::string build_path_json(bool show_results);
    void bind_add_cias();
    void bind_remove_cia();
    void bind_fix_banners();
    void bind_set_replace_files();*/

  public:
    UI(Settings &set);
    ~UI();
};
