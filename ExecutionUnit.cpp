#include "ExecutionUnit.h"

// int logic_lat = 1;
// int add_lat = 2;
// int mul_lat = 4;
// int div_lat = 5;
// int mem_lat = 4;

// int logic_rs_size = 4;
// int adder_rs_size = 4;
// int mult_rs_size = 2;
// int div_rs_size = 2;
// int br_rs_size = 2;
// int lsq_rs_size = 32;

ExecutionUnit::ExecutionUnit(UnitType name, int latency, int RS_size):myRS(RS_size, latency, 1){ //if we have non pipelined instructions this would be name in S? latency : 1, where S is the set of Non pipelined execution units.
//pipeline wise constructor to the Reservation Station
//initialise name, latency.
this->name=name;
this->latency=latency;
RS myRS();
}

//add
//mul 
//div
//branching
//bitwise operations: the unit entries take a different form?
//memory unit.

//process: 
//1. execute: returns ROB tag and the value its register has gotten. -1 if nothing to return
//doesnt push anything yet.// also broadcast via the return.
std::pair<int, int> ExecutionUnit::executeCycle(){//arguments: none, return int: ROB tag, val: the result of the calculation. 
// update_rs(), returns index.
int idx = myRS.step_rs_get_final();//index of the entry that has completed the pipeline.
if(idx!=-1){
    int src1 = myRS.get_src1_at(idx);
    int src2 = myRS.get_src2_at(idx);
    int tag = myRS.get_ROB_at(idx);
    OpCode op = myRS.get_op_at(idx);
    
    // enum class OpCode { ADD, SUB, ADDI, MUL, DIV, REM, LW, SW, BEQ, BNE, BLT, BLE, J, SLT, SLTI, AND, OR, XOR, ANDI, ORI, XORI };
    if(name== UnitType::ADDER){//ADD, SUB, ADDI
        if(op == OpCode::ADD)
        return (std::make_pair(tag, add(src1, src2)));
        else if(op == OpCode::ADDI)
        return (std::make_pair(tag, (src1 - src2)));//what is the LOGIC FOR IMMEDIATES?
        else //if(op == OpCode::ADDI)
        return (std::make_pair(tag, add(src1, src2)));
    }
    else if(name==UnitType::MULTIPLIER){//MUL
        if(op == OpCode::MUL)
        return (std::make_pair(tag, mul(src1, src2)));
    }
    else if(name==UnitType::DIVIDER){//DIV, REM
        if(op == OpCode::DIV)
        return (std::make_pair(tag, div(src1, src2)));
        else if(op == OpCode::REM)
        return (std::make_pair(tag, (src1 % src2)));
    }
    else if(name == UnitType::BRANCH){// BEQ, BNE, BLT, BLE, J
    }
    else if(name == UnitType::LOADSTORE){//LW, SW  
    }
    else{//(name == UnitType::LOGIC)  SLT, SLTI, AND, OR, XOR, ANDI, ORI, XORI  IMMEDIATE HANDLING IS AGAIN NECESSARY.
        if(op == OpCode::AND)
        return (std::make_pair(tag, (src1 & src2)));
        else if(op == OpCode::OR)
        return (std::make_pair(tag, (src1 | src2)));
        else if(op == OpCode::XOR)
        return (std::make_pair(tag, (src1 ^ src2)));
        else if(op == OpCode::ANDI)
        return (std::make_pair(tag, (src1 & src2)));
        else if(op == OpCode::ORI)
        return (std::make_pair(tag, (src1 | src2)));
        else if(op == OpCode::XORI)
        return (std::make_pair(tag, (src1 ^ src2)));
        else if(op == OpCode::SLT)
        return (std::make_pair(tag, int (src1 < src2)));
        else if(op == OpCode::SLTI)
        return (std::make_pair(tag, int (src1 < src2)));
    }

    // delete entry from RS
    myRS.invalidate_entry(idx);
}
else{
    return std::make_pair(-1,-1);
}
}
//boadcast: return tag and value, store in a vector for all execution units and then iterate through the array and call the captures of all necessary place.


void ExecutionUnit::capture (int tag, int val)
{
    //look in RS wherever the tag exists and replace it at those indices.
    myRS.capture(tag, val);
}