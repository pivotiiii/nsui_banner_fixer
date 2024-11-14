#include <format>
#include <sstream>

#include "ArgsParser.hpp"
#include "Game.hpp"
#include "UI.hpp"

#include <embedded/all.hpp>

#include "tinyfiledialogs.h"

namespace fs = std::filesystem;

bool containsOnlyASCII(const std::string &filePath)
{
    for (auto c : filePath) {
        if (static_cast<unsigned char>(c) > 127) {
            return false;
        }
    }
    return true;
}

UI::UI(Settings &set)
    : app {saucer::application::acquire({.id = "nsui-banner-fixer"})},
      smartview {{.application = app,
                  .persistent_cookies = false}}
{
    this->set = set;
    this->set.replace = false;
    std::vector<fs::path> cia_paths;
    get_cia_files("", cia_paths);
    for (const auto &path : cia_paths) {
        cia_files.push_back(Cia_File(path));
    }

    this->smartview.set_title("NSUI Banner Fixer");
    this->smartview.set_decorations(false);
    this->smartview.set_context_menu(false);
    this->smartview.set_min_size(460, 425);
    this->smartview.set_size(800, 550);

    this->smartview.set_dev_tools(true);

    this->smartview.embed(saucer::embedded::all());

    auto icon = saucer::icon::from(saucer::embedded::all().at("icon2.ico").content);
    this->smartview.set_icon(icon.value());
    this->smartview.expose("quit", [&]() {
        this->app->quit();
    });

    this->smartview.expose("maximize", [&]() {
        this->smartview.set_maximized(true);
    });

    this->smartview.expose("not_maximize", [&]() {
        this->smartview.set_maximized(false);
    });

    this->smartview.expose("minimize", [&]() {
        this->smartview.set_minimized(true);
    });

    this->bind_add_cias();
    this->bind_remove_cia();
    this->bind_set_replace_files();
    this->bind_fix_banners();

    this->smartview.serve("index.html");
    this->smartview.show();

    this->app->run();
}

UI::~UI()
{
}

void UI::add_path(const fs::path &path)
{
    if (!std::any_of(cia_files.begin(), cia_files.end(), [&path](const Cia_File &item) { return item.path == path; })) {
        cia_files.push_back(Cia_File(path, false));
    }
}

void UI::remove_path(const fs::path &path)
{
    std::erase_if(cia_files, [&path](const Cia_File &item) { return item.path == path; });
}

void UI::remove_path(const unsigned int &index)
{
    cia_files.erase(cia_files.begin() + index);
}

std::vector<fs::path> UI::split_into_paths(const std::string &str, char delimiter)
{
    std::vector<fs::path> tokens;
    std::string token;
    std::stringstream ss(str);

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(fs::path(token));
    }

    return tokens;
}

std::string UI::build_path_json(bool show_results = false)
{
    std::string files, result_bools_string, messages;
    for (const auto &result : cia_files) {
        files = files + "\"" + result.path.filename().string() + "\"" + ",";
        result_bools_string = result_bools_string + (result.result ? "true" : "false") + ",";
        messages = messages + "\"" + result.message + "\"" + ",";
    }
    files = files.substr(0, files.size() - 1);
    result_bools_string = result_bools_string.substr(0, result_bools_string.size() - 1);
    messages = messages.substr(0, messages.size() - 1);

    std::string retVal = std::format("{{\"showResults\": {}, \"files\": [{}], \"results\": [{}], \"messages\": [{}]}}", (show_results ? "true" : "false"), files, result_bools_string, messages);
    return retVal;
}

void UI::bind_add_cias()
{

    this->smartview.expose("add_cias", [&](const std::string &req = "") -> std::string {
        std::vector<const char*> file_terminators = {"*.cia"};
        const char* output = tinyfd_openFileDialog(
            "Select .cia files to fix.",
            "",
            file_terminators.size(),
            file_terminators.data(),
            ".cia files",
            1);
        if (output != NULL) {
            std::string selected_files(output);
            // std::replace(selected_files.begin(), selected_files.end(), '\\', '/');
            std::vector<fs::path> selected_file_paths = split_into_paths(selected_files, '|');
            for (const auto &path : selected_file_paths) {
                add_path(path);
            }
        }
        return build_path_json();
    });
}

void UI::bind_remove_cia()
{
    /*this->smartview.expose("remove_cia", [&](const std::string &req) -> std::string {
        remove_path(std::stoi(req.substr(1, req.size() - 1)));
        return build_path_json();
    });*/
    this->smartview.expose("remove_cia", [&](const int &req) -> std::string {
        remove_path(req);
        return build_path_json();
    });
}

void UI::bind_set_replace_files()
{
    this->smartview.expose("set_replace_files", [&](const bool &replace) {
        this->set.replace = replace;
    });
}

void UI::bind_fix_banners()
{
    this->smartview.expose("fix_banners", [&]() -> std::string {
        std::vector<Cia_File> results;

        for (const Cia_File &file : this->cia_files) {
            auto result = fix_cia(file.path, this->set);

            if (result.result == false && !containsOnlyASCII(result.path.string())) {
                result.message = "ASCII Error";
            } else if (result.result == false) {
                result.message = "not v28 Error";
            }

            results.push_back(result);
        }
        this->cia_files = results;

        for (const auto &res : results) {
            this->smartview.execute("console.log(\"wowza\")");
            this->smartview.execute("console.log(" + res.message + ")");
        }

        return build_path_json(true);

        // add error messages

        std::string files, result_bools_string, messages;
        for (const auto &result : results) {
            files = files + "\"" + result.path.filename().string() + "\"" + ",";
            result_bools_string = result_bools_string + (result.result ? "true" : "false") + ",";
            messages = messages + "\"" + result.message + "\"" + ",";
        }
        files = files.substr(0, files.size() - 1);
        result_bools_string = result_bools_string.substr(0, result_bools_string.size() - 1);
        messages = messages.substr(0, messages.size() - 1);

        std::string retVal = std::format("{{\"files\": [{}], \"results\": [{}], \"messages\": [{}]}}", files, result_bools_string, messages);

        return retVal;
    });
}
