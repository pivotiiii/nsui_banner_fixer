#pragma once

#include <filesystem>
#include <mutex>
#include <vector>

#include "webview.h"

#include "Settings.hpp"
#include "nsui_banner_fixer.hpp"

class UI {
  private:
    std::unique_ptr<webview::webview> wp;
    Settings set;
    std::vector<Cia_File> cia_files;
    std::string css;
    std::string script;
    std::string html_nsui;
    std::string html_license;

    std::string build_final_html(std::string &html);
    void load_nsui_page();
    void add_path(const std::filesystem::path &path);
    void remove_path(const std::filesystem::path &path);
    void remove_path(const unsigned int &index);
    std::vector<std::filesystem::path> split_into_paths(const std::string &str, char delimiter);
    std::string build_path_json(bool show_results);
    void bind_add_cias();
    void bind_remove_cia();
    void bind_fix_banners();
    void bind_set_replace_files();

  public:
    UI(Settings &set);
    ~UI();
};
