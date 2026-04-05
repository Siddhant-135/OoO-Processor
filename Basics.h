#pragma once
#include <string>

enum class OpCode { ADD, SUB, ADDI, MUL, DIV, REM, LW, SW, BEQ, BNE, BLT, BLE, J, SLT, SLTI, AND, OR, XOR, ANDI, ORI, XORI };
enum class UnitType { ADDER, MULTIPLIER, DIVIDER, LOADSTORE, BRANCH, LOGIC };

struct Instruction {
    OpCode op;
    int dest;
    int src1;
    int src2;
    int imm;
    int pc;
};

struct ProcessorConfig {
    int num_regs = 32;
    int rob_size = 64;
    int mem_size = 1024;

    int logic_lat = 1;
    int add_lat = 2;
    int mul_lat = 4;
    int div_lat = 5;
    int mem_lat = 4;

    int logic_rs_size = 4;
    int adder_rs_size = 4;
    int mult_rs_size = 2;
    int div_rs_size = 2;
    int br_rs_size = 2;
    int lsq_rs_size = 32;
};

struct ROBEntry {
    // valid bit, ready bit, architectural register ID
    // other fields as required
    bool valid = false;
    bool ready_from_RS= false;//always ready to go to RS. This is instead bool for "has it received a value from RS" 
    bool sent_to_RS = true; //if the reservation station was full. Could have to change this and stall the whole pipeline.
    int ROBid; //necessary? SInce we'll be  iterating idts.
    int dest_regId; //just store the dest_regVal. What about memory?
    int dest_regVal;
    int src1;
    int src2;
    OpCode op;
};

struct RSEntry {
    // value, tag, ready ... for both operands
    // other fields as required
    //3 resevation stations: add, mul, div.
    bool src1_valid, src2_valid;
    int src1_tag, src2_tag, src1_value, src2_value;
    OpCode op;
    //index of RSEntry?
};