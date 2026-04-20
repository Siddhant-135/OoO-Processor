#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string_view>
#include "Basics.h"
#include "parser/Parser.h"
#include "BranchPredictor/BranchPredictor.h"
#include "execute_units/ExecutionUnit.h"
#include "decode_units/LoadStoreQueue.h"
#include "decode_units/RegisterAliasTable.h"
#include "decode_units/ReorderBuffer.h"
#include "execute_units/ExecutionUnit.h"

struct Pipeline_reg{
    Instruction inst;
    BP_info bp_info;
    bool valid = false; // to control the 1st inst.
};

class Processor {
private:
    // Snapshot before execute each cycle: issue (decode) must not see RS slots
    // freed by completions in the same cycle as decode runs after execute in step().
    std::vector<bool> rs_full_before_execute;
public:
    int pc;
    int clock_cycle;
    int dispatch_counter = 0; // monotonic counter for oldest-first RS dispatch
    bool flushed_this_cycle = false;
    bool verbose = false;

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
    // LoadStoreQueue* lsq;
    BranchPredictor bp;

    // Parser instance
    Parser myparser;

    Processor(ProcessorConfig& config);

    void loadProgram(const std::string& filename);

    void flush();

    void broadcastOnCDB( std::vector<ExuResult> b_vec);

    int getUnitIdx(OpCode op);

    void stageFetch();

    void stageDecode();

    void stageExecuteAndBroadcast();

    void stageCommit();

    bool step();

    void setVerbose(bool v) {
        verbose = v;
        for (auto& unit : units) {
            unit.setVerbose(v);
        }
    }

    void dumpArchitecturalState() {
        std::cout << "\n=== ARCHITECTURAL STATE (CYCLE " << clock_cycle << ") ===\n";
        for (size_t i = 0; i < ARF.size(); i++) {
            std::cout << "x" << i << ": " << std::setw(4) << ARF[i] << " | ";
            if ((i+1) % 8 == 0) std::cout << std::endl;
        }
        if (exception) {
            std::cout << "EXCEPTION raised by instruction " << pc + 1 << std::endl;
        }
        std::cout << "Branch Predictor Stats: " << bp.correct_predictions << "/" << bp.total_branches << " correct.\n";
    }
};
