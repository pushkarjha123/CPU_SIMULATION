#include "Apex_Pipeline_Definition.hpp"

int main (int argc, char **argv) {
    if (argc == 2) {
        InstructionCodeFile = argv[1];
    } else if (argc == 3) {
        InstructionCodeFile = argv[1];
        _PRI = atoi(argv[2]);
    } else {
        write_log(1,"invalid inputs\n");
        exit(0);
    }
    shell();
}
