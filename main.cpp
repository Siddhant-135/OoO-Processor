#include <iostream>
#include <string>
#include "Processor.h"

using namespace std;
int main(int argc, char* argv[]) {

    if (argc < 2) {
        cerr << "Usage: ./main <filename.s> [-cycles N]\n";
        return 1;
    }

    int max_cycles = -1; 
    if (argc == 4 && string(argv[2]) == "-cycles") {
        max_cycles = stoi(argv[3]);
    }

    ProcessorConfig config;
    Processor cpu = Processor(config);
    
    try {
        cpu.loadProgram(argv[1]);
    } catch (...) {
        cerr << "Failed to parse instruction file.\n";
        return 1;
    }

    int cycle_count = 0;
    while (cpu.step()) {
        cycle_count++;
        if (max_cycles != -1 && cycle_count == max_cycles) {
            cout << "\n[!] Execution halted at cycle limit: " << max_cycles << "\n";
            break;
        }
        else if (max_cycles == -1 && cycle_count >= 10000) // Extra added just as a safeguard, remove for full length functionality.
        {
            cout << "\n[!] Execution halted at 10,000 cycles to prevent bad things from happening, remove from main.cpp " << "\n";
            break;
        }
    }

    if (max_cycles == -1) {
        if (cpu.exception) {
            cout << "\n[+] Execution halted due to exception after " << cpu.clock_cycle << " cycles.\n";
        }
        else {
            cout << "\n[+] Execution complete naturally in " << cpu.clock_cycle << " cycles.\n";
        }
    }

    cpu.dumpArchitecturalState();
    for (int i=0;i<cpu.Memory.size();i++) {
        cout << cpu.Memory[i] << " ";
    }
    cout << endl;
    return 0;
}
