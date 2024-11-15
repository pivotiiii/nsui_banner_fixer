#include <format>

#include "Game.hpp"
#include "UI.hpp"

#include <embedded/all.hpp>

#include "tinyfiledialogs.h"

#ifndef VERSION
#define VERSION "0.0.0"
#endif
#ifndef YEAR
#define YEAR "0000"
#endif
#ifndef COMPILE_TIME
#define COMPILE_TIME "0000-00-00 00:00:00 UTC"
#endif

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

std::string replace_char_with_string(const std::string &str, char replaced, const std::string &replacement)
{
    std::string result = str;
    std::size_t pos = str.find(replaced);
    while (pos != std::string::npos) {
        result.replace(pos, 1, replacement);                     // Replace the character with the replacement string
        pos = result.find(replaced, pos + replacement.length()); // Find the next occurrence
    }
    return result;
}

UI::UI(Settings &set)
    : app {saucer::application::acquire({.id = "nsui-banner-fixer"})},
      smartview {{.application = app,
                  .persistent_cookies = false}}
{
    this->set = set;
    this->set.replace = false;

    this->smartview.set_title("NSUI Banner Fixer");
    this->smartview.set_decorations(false);
    this->smartview.set_context_menu(false);
    this->smartview.set_min_size(460, 425);
    this->smartview.set_size(800, 550);

    this->smartview.set_dev_tools(true);

    this->smartview.embed(saucer::embedded::all());

    auto icon = saucer::icon::from(saucer::embedded::all().at("icon2.ico").content);
    this->smartview.set_icon(icon.value());

    auto license_data = saucer::embedded::all().at("LICENSE.txt").content.data();
    this->license = reinterpret_cast<const char*>(license_data);

    this->expose_functions();

    this->smartview.serve("index.html");
    this->smartview.show();

    this->app->run();
}

UI::~UI()
{
}

void UI::expose_functions()
{
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

    this->smartview.expose("check_requirements", [&]() -> std::string {
        return this->check_requirements();
    });

    this->smartview.expose("add_cias", [&]() -> std::string {
        return this->add_cias();
    });

    this->smartview.expose("remove_cia", [&](const int &req) -> std::string {
        return this->remove_cia(req);
    });

    this->smartview.expose("set_replace_files", [&](const bool &replace) {
        this->set_replace_files(replace);
    });

    this->smartview.expose("fix_banners", [&]() -> std::string {
        return this->fix_banners();
    });

    this->smartview.expose("get_program_info", [&]() -> std::string {
        return this->get_program_info();
    });
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

std::string UI::add_cias()
{
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
}

std::string UI::remove_cia(const int &req)
{
    remove_path(req);
    return build_path_json();
}

void UI::set_replace_files(const bool &replace)
{
    this->set.replace = replace;
}

std::string UI::fix_banners()
{
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
}

std::string UI::check_requirements()
{
    std::vector<fs::path> reqs {set.dstool, set.ctrtool, set.makerom};
    uint8_t err_count = 0;
    std::string missing_files;
    for (const fs::path &path : reqs) {
        if (!fs::exists(path)) {
            missing_files = missing_files + "\"" + path.filename().string() + "\",";
            err_count = err_count + 1;
        }
    }
    missing_files = missing_files.substr(0, missing_files.size() - 1);
    std::string retVal = std::format("{{\"result\": {}, \"err_count\": {}, \"missing_files\": [{}]}}", (err_count == 0 ? "true" : "false"), err_count, missing_files);
    return retVal;
}

std::string UI::get_program_info()
{
    std::string license_text = this->license;
    license_text = replace_char_with_string(license_text, '\r', "");
    license_text = replace_char_with_string(license_text, '\n', "<br>");
    license_text = replace_char_with_string(license_text, '"', "\\\"");

    std::string retVal = std::format("{{\"license\": \"{}\", \"compile_time\": \"{}\", \"year\": \"{}\", \"version\": \"{}\"}}", license_text, COMPILE_TIME, YEAR, VERSION);
    return retVal;
}
