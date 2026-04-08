#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string_view>
#include "Basics.h"
#include "Parser.h"
#include "BranchPredictor.h"
#include "ExecutionUnit.h"
#include "LoadStoreQueue.h"
#include "RegisterAliasTable.h"
#include "ReorderBuffer.h"
#include "ExecutionUnit.h"

struct Pipeline_reg{
    Instruction inst;
};

class Processor {
private:
public:
    int pc;
    int clock_cycle;

    // pipeline registers
    Pipeline_reg F_reg;
    Pipeline_reg D_reg;

    std::vector<Instruction> inst_memory;

    // architectural state (do not change)
    std::vector<int> ARF; // regFile
    std::vector<int> Memory; // Memory
    bool exception = false; // exception bit

    // register alias table / reorder buffer
    ROB myROB;
    RAT myRAT;
    std::vector<ExecutionUnit> units;
    LoadStoreQueue* lsq;
    BranchPredictor bp;

    // Parser instance
    Parser myparser;



    Processor(ProcessorConfig& config);

    void loadProgram(const std::string& filename);

    void flush();

    void broadcastOnCDB( std::vector<std::pair<int,int>> b_vec);

    int getUnitIdx(OpCode op);

    void stageFetch();

    void stageDecode();

    void stageExecuteAndBroadcast();

    void stageCommit();

    bool step();

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