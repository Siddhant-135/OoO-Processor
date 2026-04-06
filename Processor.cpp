#include "Processor.h"

Processor::Processor(ProcessorConfig& config) :myROB(config.rob_size){
    pc = 0;
    clock_cycle = 0;
    ARF.resize(config.num_regs, 0);
    Memory.resize(config.mem_size, 0);
    Parser myparser = Parser();
    RAT myRAT();
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
    myparser.parseFile(file, Processor::inst_memory, Processor::Memory);
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
    }

};

void Processor::stageDecode() {
    //we think decode never stalls.
    D_reg.inst = F_reg.inst;
    if(myROB.is_Full() /*add RAT fulll condition*/){
        //stall
        return;
    }
    else{
        myROB.push(D_reg.inst);
        //push to appropriate reservation station too.
    }
};

void Processor::stageExecuteAndBroadcast() {};

void Processor::stageCommit() {
    int idx = myROB.dest_reg();
    ARF[idx] = myROB.dest_val();
    myROB.pop();
    myRAT.rem_from_RAT(idx);
};

bool Processor::step() {
    clock_cycle++;
    if (exception) 
    {
        flush();
        return false;
    }
    else return true; // return false if CPU has no more to do after this cycle
}