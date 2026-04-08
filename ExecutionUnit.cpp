#include "ExecutionUnit.h"

// int logic_lat = 1; add_lat = 2; mul_lat = 4; div_lat = 5; mem_lat = 4;

// int logic_rs_size = 4; adder_rs_size = 4; int mult_rs_size = 2; int div_rs_size = 2; int br_rs_size = 2; 
// int lsq_rs_size = 32;

ExecutionUnit::ExecutionUnit(UnitType name, int latency, int RS_size, std::vector<int>& memory): memory(memory), myRS(RS_size, latency, 1) { 
this->name=name;
this->latency=latency;
}

UnitType ExecutionUnit::getUnitTypeForOp(OpCode op) {
    switch (op) {
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::ADDI:
            return UnitType::ADDER;
        case OpCode::MUL:
            return UnitType::MULTIPLIER;
        case OpCode::DIV:
        case OpCode::REM:
            return UnitType::DIVIDER;
        case OpCode::BEQ:
        case OpCode::BNE:
        case OpCode::BLT:
        case OpCode::BLE:
        case OpCode::J:
            return UnitType::BRANCH;
        case OpCode::LW:
        case OpCode::SW:
            return UnitType::LOADSTORE;
        case OpCode::SLT:
        case OpCode::SLTI:
        case OpCode::AND:
        case OpCode::OR:
        case OpCode::XOR:
        case OpCode::ANDI:
        case OpCode::ORI:
        case OpCode::XORI:
            return UnitType::LOGIC;
    }
    return UnitType::ADDER;
}

//returns ROB tag and the value its register has gotten. -1 if nothing to return
//doesnt push anything yet.
// also broadcast via the return.

std::pair<int, int> ExecutionUnit::executeCycle(){ //arguments: none, return int: ROB tag, val: the result of the calculation. 
int idx = myRS.update_rs(); //returns index of the entry that has completed the pipeline.
if(idx!=-1){
    const RSEntry& rs_entry = myRS.get_entry(idx);
    int src1 = rs_entry.src1_value;
    int src2 = rs_entry.src2_value;
    int imm = rs_entry.imm_value; 
    int dest = rs_entry.dest_value;
    int tag = rs_entry.ROB_Entry;
    OpCode op = rs_entry.op;
    int output = 0;
    
    // OpCode is the operation, UnitType is the hardware doing the operation.
    if(name== UnitType::ADDER){//ADD, SUB, ADDI
        if(op == OpCode::ADD)
        output = add(src1, src2);
        else if(op == OpCode::SUB)
        output = (src1 - src2);
        else //(ADDI)
        output = add(src1, imm);
    }
    else if(name==UnitType::MULTIPLIER){//MUL
        if(op == OpCode::MUL)
        output = mul(src1, src2);
    }
    else if(name==UnitType::DIVIDER){//DIV, REM
        if(op == OpCode::DIV)
        output = div(src1, src2);
        else if(op == OpCode::REM)
        output = (src1 % src2);
    }
    else if(name == UnitType::BRANCH){// BEQ, BNE, BLT, BLE, J
        if(op == OpCode::BEQ)
        output = int(src1 == src2);
        else if(op == OpCode::BNE)
        output = int(src1 != src2);
        else if(op == OpCode::BLT)
        output = int(src1 < src2);
        else if(op == OpCode::BLE)
        output = int(src1 <= src2);
        else if(op == OpCode::J)
        output = imm; 
    }
    else if(name == UnitType::LOADSTORE){//LW, SW  
        if(op == OpCode::LW)
        output = memory[src1 + imm]; 
        else if(op == OpCode::SW)
        output = memory[dest + imm];
    }
    else{//(name == UnitType::LOGIC)  SLT, SLTI, AND, OR, XOR, ANDI, ORI, XORI  IMMEDIATE HANDLING IS AGAIN NECESSARY.
        if(op == OpCode::AND)
        output = (src1 & src2);
        else if(op == OpCode::OR)
        output = (src1 | src2);
        else if(op == OpCode::XOR)
        output = (src1 ^ src2);
        else if(op == OpCode::ANDI)
        output = (src1 & src2);
        else if(op == OpCode::ORI)
        output = (src1 | src2);
        else if(op == OpCode::XORI)
        output = (src1 ^ src2);
        else if(op == OpCode::SLT)
        output = int (src1 < src2);
        else if(op == OpCode::SLTI)
        output = int (src1 < src2);
    }
    return std::make_pair(tag, output); 
    myRS.invalidate_entry(idx);
}
else{
    return std::make_pair(-1,-1);
}
}

void ExecutionUnit::capture (int tag, int val)
{
    myRS.capture(tag, val);
}
