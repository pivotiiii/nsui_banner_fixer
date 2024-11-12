#include <format>
#include <sstream>

#include "ArgsParser.hpp"
#include "Game.hpp"
#include "UI.hpp"

#include <embedded/all.hpp>

#include "tinyfiledialogs.h"

namespace fs = std::filesystem;

UI::UI(Settings &set)
    : app {saucer::application::acquire({.id = "nsui-banner-fixer"})},
      smartview {{.application = app}}
{
    this->set = set;
    this->set.replace = false;
    std::vector<fs::path> cia_paths;
    get_cia_files("", cia_paths);
    for (const auto &path : cia_paths) {
        cia_files.push_back(Cia_File(path));
    }

    smartview.set_title("NSUI Banner Fixer");
    smartview.set_decorations(false);
    smartview.set_context_menu(false);
    smartview.set_min_size(460, 350);
    smartview.set_size(800, 550);

    smartview.set_dev_tools(true);

    smartview.embed(saucer::embedded::all());

    smartview.expose("quit", [&]() {
        this->app->quit();
    });

    smartview.expose("maximize", [&]() {
        smartview.set_maximized(true);
    });

    smartview.expose("not_maximize", [&]() {
        smartview.set_maximized(false);
    });

    smartview.expose("minimize", [&]() {
        smartview.set_minimized(true);
    });

    smartview.serve("index.html");
    smartview.show();
    smartview.execute("console.log({})", std::vector<int> {10});
    app->run();
}

UI::~UI()
{
}

void UI::quit()
{
    this->app->quit();
}

/*UI::UI(Settings &set)
{
    this->set = set;
    this->set.replace = false;
    std::vector<fs::path> cia_paths;
    get_cia_files("", cia_paths);
    for (const auto &path : cia_paths) {
        cia_files.push_back(Cia_File(path));
    }

    std::string css_style = b::embed<"ui/windows-ui/windows-ui.min.css">();
    std::string css_icons = b::embed<"ui/windows-ui/winui-icons.min.css">();
    std::string css_config = b::embed<"ui/windows-ui/config/app-config.css">();
    std::string css_overwrites = b::embed<"ui/style.css">();
    std::string js_ui = ""; // b::embed<"ui/windows-ui/windows-ui.min.js">();
    std::string js = b::embed<"ui/script.js">();

    css = "<style>" + css_config + css_icons + css_style + css_overwrites + "</style>";
    script = "<script>" + js + js_ui + "</script>";

    html_nsui = b::embed<"ui/index.html">();
    html_nsui = build_final_html(html_nsui);

    wp = std::make_unique<webview::webview>(false, nullptr);
    wp->set_title("NSUI Banner Fixer");

    bind_add_cias();
    bind_remove_cia();
    bind_fix_banners();
    bind_set_replace_files();

    load_nsui_page();
    wp->set_size(400, 400, WEBVIEW_HINT_MIN);

    wp->run();
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

std::string UI::build_final_html(std::string &html)
{
    const std::string style_placeholder = "<!--STYLE-->";
    std::string html_out = html.replace(html.find(style_placeholder), style_placeholder.length(), this->css);
    const std::string script_placeholder = "<!--SCRIPT-->";
    html_out = html_out.replace(html_out.find(script_placeholder), script_placeholder.length(), script);
    return html_out;
}

void UI::load_nsui_page()
{
    wp->set_html(html_nsui);
}

void UI::bind_add_cias()
{

    wp->bind("add_cias", [&](const std::string &req = "") -> std::string {
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
    wp->bind("remove_cia", [&](const std::string &req) -> std::string {
        remove_path(std::stoi(req.substr(1, req.size() - 1)));
        return build_path_json();
    });
}

void UI::bind_set_replace_files()
{
    wp->bind("set_replace_files", [&](const std::string &replace) -> std::string {
        if (replace.substr(1, replace.size() - 2) == "true") {
            this->set.replace = true;
        } else {
            this->set.replace = false;
        }
        return "";
    });
}

void UI::bind_fix_banners()
{
    wp->bind("fix_banners", [&](const std::string &replace) -> std::string {
        std::vector<Cia_File> results;

        for (const Cia_File &file : this->cia_files) {
            results.push_back(fix_cia(file.path, this->set));
        }
        this->cia_files = results;

        return build_path_json(true);

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
}*/
