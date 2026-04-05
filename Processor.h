#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "Basics.h"
#include "BranchPredictor.h"
#include "ExecutionUnit.h"
#include "LoadStoreQueue.h"

struct Pipeline_reg{
    Instruction inst;
    bool stall;
};


class Processor {
private:
    Instruction lineToInst(std::string line)
    {
        
    }
public:
    int pc;
    int clock_cycle;

    // pipeline registers

    Pipeline_reg F_reg();
    Pipeline_reg D_reg();

    std::vector<Instruction> inst_memory;

    // architectural state (do not change)
    std::vector<int> ARF; // regFile
    std::vector<int> Memory; // Memory
    bool exception = false; // exception bit

    // register alias table / reorder buffer

    std::vector<ExecutionUnit> units;
    LoadStoreQueue* lsq;
    BranchPredictor bp;

    Processor(ProcessorConfig& config) {};

    void loadProgram(const std::string& filename) {};

    void flush() {};

    void broadcastOnCDB() {};

    void stageFetch() {
        //handle the stall of a previous decode instruction.
        if(D_reg.stall)
        pc ++;
        //check instruction memory for the presence of instructions, DONE
        if(pc == inst_memory.size())
        return;
        
        F_reg = inst_memory[pc];
    };

    void stageDecode() {};

    void stageExecuteAndBroadcast() {};

    void stageCommit() {};

    bool step() {
    //we dont always have to update the pc though, so PC updation should happen conditionally, in stageFetch.
        

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