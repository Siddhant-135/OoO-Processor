#include "ExecutionUnit.h"

// int logic_lat = 1; add_lat = 2; mul_lat = 4; div_lat = 5; mem_lat = 4;

// int logic_rs_size = 4; adder_rs_size = 4; int mult_rs_size = 2; int div_rs_size = 2; int br_rs_size = 2; 
// int lsq_rs_size = 32;

ExecutionUnit::ExecutionUnit(UnitType name, int latency, int RS_size, std::vector<int>& memory): memory(memory){ 
this->name=name;
this->latency=latency;

if(name == UnitType::LOADSTORE){
    myRS = std::unique_ptr<RS>(std::make_unique<LoadStoreQueue>(RS_size, latency, 1));
} else {
    myRS = std::unique_ptr<RS>(std::make_unique<RS>(RS_size, latency, 1)); 
}
}

bool ExecutionUnit::check_output_bound(long long value) {
    return (value >= MIN_N && value <= MAX_N);
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

ExuResult ExecutionUnit::executeCycle(){ //arguments: none, return int: ROB tag, val: the result of the calculation. 
int idx = myRS->update_rs(); //returns index of the entry that has completed the pipeline.
ExuResult result;
if(idx!=-1){
    const RSEntry& rs_entry = myRS->get_entry(idx);
    int src1 = rs_entry.src1_value; 
    int src2 = rs_entry.src2_value;
    int imm = rs_entry.imm_value; 
    int tag = rs_entry.ROB_Entry;
    OpCode op = rs_entry.op;
    long long temp_output = 0;
    int output = 0;
    
    // OpCode is the operation, UnitType is the hardware doing the operation.
    if(name== UnitType::ADDER){//ADD, SUB, ADDI, // TO ADD: SLTI and SLT in ADDER
        if(op == OpCode::ADD)
        temp_output = add(src1, src2);
        else if(op == OpCode::SUB)
        temp_output = add(src1, -src2);
        else if(op == OpCode::ADDI)
        temp_output = add(src1, imm);
        else if(op == OpCode::SLTI)
        temp_output = add(src1, -imm);
        else if(op == OpCode::SLT)
        temp_output = add(src1, -src2);

        if(!check_output_bound(temp_output)) {
            result.has_exception = true;
            output = 0;
            std::cout << "    Adder overflow for ROB tag " << tag << "\n";
        } 
        else {
            if (op == OpCode::SLTI || op == OpCode::SLT)
            output = int(temp_output < 0);
            else
            output = static_cast<int>(temp_output);
        }
    }
    
    else if(name==UnitType::MULTIPLIER){//MUL
        temp_output = mul(src1, src2);
        if(!check_output_bound(temp_output)) {
            result.has_exception = true;
            output = 0;
            std::cout << "    Multiplier overflow for ROB tag " << tag << "\n";
        }
        else {output = static_cast<int>(temp_output);}
    }
    else if(name==UnitType::DIVIDER){//DIV, REM
        if(src2 == 0){
            result.has_exception = true;
            output = 0;
            std::cout << "    Divider exception: divide/remainder by zero for ROB tag " << tag << "\n";
        }
        else if(op == OpCode::DIV)
        temp_output = div(src1, src2);
        else if(op == OpCode::REM)
        temp_output = (src1 % src2);
        if(!check_output_bound(temp_output)) {
            result.has_exception = true;
            output = 0;
            std::cout << "    Divider overflow for ROB tag " << tag << "\n";
        }
        else {output = static_cast<int>(temp_output);}
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
    else if(name == UnitType::LOADSTORE){//LW, SW  forwarding needs to be added in get ready entry.
        int mem_addr = (op == OpCode::LW) ? (src1 + imm) : (src2 + imm);
        bool out_of_bounds = (mem_addr < 0 || mem_addr >= static_cast<int>(memory.size()));
        if(op == OpCode::LW){
            if(out_of_bounds){
                result.has_exception = true;
                output = 0;
                std::cout << "    Memory exception: LW address out of bounds (" << mem_addr
                          << ") for ROB tag " << tag << "\n";
            }
            else if(rs_entry.ls_fwded){//can also change to mem_valid but okay.
                output = rs_entry.mem_val;
            }
            else{
                output = memory[mem_addr];
            }
        }
        else if(op == OpCode::SW){
            if(out_of_bounds){
                result.has_exception = true;
                result.mem_addr = -1;
                result.mem_val = 0;
                std::cout << "    Memory exception: SW address out of bounds (" << mem_addr
                          << ") for ROB tag " << tag << "\n";
            } else {
                result.mem_addr = mem_addr;
                result.mem_val = src1;
            }
            // No instruction has an ROB tag corresponding to a store since RAT never updates on sw, so no output needed
            // so the tag based check will happen for this instruction's ROB too: benign
        }       
    }
    else{//(name == UnitType::LOGIC)  SLT, SLTI, AND, OR, XOR, ANDI, ORI, XORI  IMMEDIATE HANDLING IS AGAIN NECESSARY.
        if(op == OpCode::AND)
        output = (src1 & src2);
        else if(op == OpCode::OR)
        output = (src1 | src2);
        else if(op == OpCode::XOR)
        output = (src1 ^ src2);
        else if(op == OpCode::ANDI)
        output = (src1 & imm);
        else if(op == OpCode::ORI)
        output = (src1 | imm);
        else if(op == OpCode::XORI)
        output = (src1 ^ imm);
        else if(op == OpCode::SLT)
        output = int (src1 < src2);
        else if(op == OpCode::SLTI)
        output = int (src1 < imm);
    }
    result.tag = tag;
    result.value = output;
    myRS->invalidate_entry(idx); 
    myRS->PipelineCounter--;
    std::cout<<"Im here!\n";
    if(myRS->PipelineCounter<latency){//see fix, divide latency by stages per instruction.
        loadToPipeline();
    }
    return result; 
}
else{
    std::cout<<"Im there, nothing ready out of pipeline yet!\n";
    if(myRS->PipelineCounter<latency){//see fix, divide latency by stages per instruction.
        loadToPipeline();
        }
    return result;
}
}

void ExecutionUnit::exu_capture (int tag, int val)
{
    std::cout<<"Execution Unit "<<static_cast<int>(name)<<" capturing on CDB: tag "<<tag<<" value "<<val<<"\n";
    myRS->rs_capture(tag, val);
}

void ExecutionUnit::reset() {
    has_result = false;
    has_exception = false;
    myRS->reset();
}
