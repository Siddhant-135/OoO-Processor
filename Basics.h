#pragma once
#include <string>

enum class OpCode { ADD, SUB, ADDI, MUL, DIV, REM, LW, SW, BEQ, BNE, BLT, BLE, J, SLT, SLTI, AND, OR, XOR, ANDI, ORI, XORI };
enum class UnitType { ADDER, MULTIPLIER, DIVIDER, LOADSTORE, BRANCH, LOGIC };
//a set of UnitTypes for nonpipelined instructions. Empty, all are pipelined

struct Instruction {
    OpCode op = OpCode::ADD; // default value, will be overwritten by parser.
    int dest = -1; // architectural register ID. -1 if no destination (eg for SW, BEQ etc).
    int src1 = -1; // architectural register ID. -1 if no source (eg for J).
    int src2 = -1;
    int imm = 0;
    int pc = 0;
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
// branch latency is adder latency
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
    bool ready_from_RS= false; //always ready to go to RS. This is instead bool for "has it received a value from RS" 
    int dest_regId = -1; 
    int dest_regVal = 0; //just store the dest_regVal. What about memory? NOT IN ROB
    int dest_memAddr = -1; // NEW FIELD AS OLD ONE WAS FOR REG ADDRESS. INTRODUCED FOR SW OPERTING AT COMMIT STAGE
    int dest_memVal = 0; // INTRODUCED FOR SW OPERTING AT COMMIT STAGE
};

struct RSEntry {
    // value, tag, ready ... for both operands
    // other fields as required
    //3 resevation stations: add, mul, div.
    bool src1_valid = false, src2_valid = false, dest_valid = false;
    int src1_tag = -1, src2_tag = -1, src1_value = -1, src2_value = -1;
    int imm_value = 0, dest_value = -1;
    int ROB_Entry = -1;
    OpCode op = OpCode::ADD;
};
