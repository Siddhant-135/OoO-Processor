#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "Basics.h"
#include "BranchPredictor.h"
#include "ExecutionUnit.h"
#include "LoadStoreQueue.h"

class Processor {
private:
    Instruction lineToInst(std::string line)
    {
        
    }
public:
    int pc;
    int clock_cycle;

    // pipeline registers

    std::vector<Instruction> inst_memory;

    // architectural state (do not change)
    std::vector<int> ARF; // regFile
    std::vector<int> Memory; // Memory
    bool exception = false; // exception bit

    // register alias table / reorder buffer

    std::vector<ExecutionUnit> units;
    LoadStoreQueue* lsq;
    BranchPredictor bp;

    Processor(ProcessorConfig& config) {
        pc = 0;
        clock_cycle = 0;
        ARF.resize(config.num_regs, 0);
        Memory.resize(config.mem_size);

        // Instantiate Hardware Units
        // Adder
        // Multiplier
        // Divider
        // Branch Computation
        // Bitwise Logic
        // Load-Store Unit
    }

    void loadProgram(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {throw std::runtime_error("corrupted file");};
        std::string line;
        while (std::getline(file, line)) 
        {
            Instruction inst = lineToInst (line);
            inst_memory.push_back(inst);
        }

    }

    void flush() {};

    void broadcastOnCDB() {};

    void stageFetch() {};

    void stageDecode() {};

    void stageExecuteAndBroadcast() {};

    void stageCommit() {};

    bool step() {
        clock_cycle++;
        if (exception) 
        {
            flush();
            return false;
        }
        else return true; // return false if CPU has no more to do after this cycle
    }

    void dumpArchitecturalState() {
        std::cout << "\n=== ARCHITECTURAL STATE (CYCLE " << clock_cycle << ") ===\n";
        for (int i = 0; i < ARF.size(); i++) {
            std::cout << "x" << i << ": " << std::setw(4) << ARF[i] << " | ";
            if ((i+1) % 8 == 0) std::cout << std::endl;
        }
        if (exception) {
            std::cout << "EXCEPTION raised by instruction " << pc + 1 << std::endl;
        }
        std::cout << "Branch Predictor Stats: " << bp.correct_predictions << "/" << bp.total_branches << " correct.\n";
    }
};