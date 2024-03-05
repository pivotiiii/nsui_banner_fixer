#include <string>
#include <vector>

#include <ctrtool/CiaProcess.h>

class Ctrtool_cia_proc {
  private:
    // std::vector<std::string> args;
    // ctrtool::Settings set;
    ctrtool::CiaProcess proc;
    std::vector<std::string> output_lines;

    

  public:
    Ctrtool_cia_proc(std::vector<std::string> &args);
    void run(bool silent);
    std::vector<std::string> get_output_lines();
};
