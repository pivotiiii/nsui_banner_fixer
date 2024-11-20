#include <filesystem>
#include <vector>

#include "Settings.hpp"

void pause_if_double_clicked(bool require_key_press = true, int sleep = 0); // fwd declared from nsui_banner_fixer.cpp
int parse_args(int argc, char** argv, std::vector<std::filesystem::path> &cias, Settings &set);
