#include <filesystem>
#include <vector>

#include "Settings.hpp"

void pause_if_double_clicked(bool require_key_press = true, int sleep = 0);
bool get_cia_files(std::string ciaArg, std::vector<std::filesystem::path> &cias);
int parse_args(int argc, char** argv, std::vector<std::filesystem::path> &cias, Settings &set);