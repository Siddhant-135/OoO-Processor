#include "Processor.h"

Processor::Processor(ProcessorConfig& config){
    pc = 0;
    clock_cycle = 0;
    ARF.resize(config.num_regs, 0);
    Memory.resize(config.mem_size, 0);
    Parser myparser = Parser();
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
    myparser.parseFile(file, Processor::inst_memory);
}

void Processor::flush() {};

void Processor::broadcastOnCDB() {};

void Processor::stageFetch() {
    pc+=1;
    //start with pc =-1
    if(pc>=inst_memory.size()){
    return;}
    else{
        F_reg.inst=inst_memory[pc];
        //does decode know that something has to be sent to it? And what? Execute the decode of the prev fetch inst before fetching new
    }

};

void Processor::stageDecode() {
    //we think decode never stalls.

    //put in reorder buffer. 
    //reorder buffer at cycle decides if the entry in it is valid or not.
};

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