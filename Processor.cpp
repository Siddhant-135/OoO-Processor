#include "Processor.h"

Processor::Processor(ProcessorConfig& config){
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

void Processor::loadProgram(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {throw std::runtime_error("corrupted file");};
    std::string line;
    while (std::getline(file, line)) 
    {
        if (line.empty() || line[0] == '#');
        Instruction inst = lineToInst (line);
        inst_memory.push_back(inst);
    }
}

void Processor::flush() {};

void Processor::broadcastOnCDB() {};

void Processor::stageFetch() {};

void Processor::stageDecode() {};

void Processor::stageExecuteAndBroadcast() {};

void Processor::stageCommit() {};

bool Processor::step() {
    clock_cycle++;
    if (exception) 
    {
        flush();
        return false;
    }
    else return true; // return false if CPU has no more to do after this cycle
}