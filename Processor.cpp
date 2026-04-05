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