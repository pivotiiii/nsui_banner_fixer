#ifndef NSUIBF_H
#define NSUIBG_H

#include <filesystem>
#include <vector>

int check_requirements(std::vector<std::filesystem::path> reqs);
int parse_args(int argc, char** argv, std::vector<std::filesystem::path> &cias, bool &replace, bool &verbose);
int main(int argc, char* argv[]);

#endif