#include "Processor.h"

Processor::Processor(ProcessorConfig& config) :myROB(config.rob_size){
    pc = 0;
    clock_cycle = 0;

    // Instantiate Hardware Units
    ARF.resize(config.num_regs, 0);
    Memory.resize(config.mem_size, 0);
    Parser myparser = Parser();
    RAT myRAT();
    
    //can I make a change to the units vector and expect it to stay?

    // std::vector<ExecutionUnit> units;
    // Adder
    units.push_back(ExecutionUnit(UnitType::ADDER, config.add_lat, config.adder_rs_size));
    // Multiplier
    units.push_back(ExecutionUnit(UnitType::MULTIPLIER, config.mul_lat, config.mult_rs_size));
    // Divider
    units.push_back(ExecutionUnit(UnitType::DIVIDER, config.div_lat, config.div_rs_size));
    // Branch Computation: see what kind of stuff the parser does.
    units.push_back(ExecutionUnit(UnitType::BRANCH, config.add_lat, config.div_rs_size));
    // Bitwise Logic
    units.push_back(ExecutionUnit(UnitType::LOGIC, config.logic_lat, config.logic_rs_size));
    // Load-Store Unit
    units.push_back(ExecutionUnit(UnitType::LOADSTORE, config.mem_lat, config.lsq_rs_size));    
}

void Processor::loadProgram(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {throw std::runtime_error("corrupted file");};
    myparser.parseFile(file, Processor::inst_memory, Processor::Memory);
}

void Processor::flush() {};

void Processor::broadcastOnCDB( std::vector<std::pair<int,int>> b_vec){
    //each ROB entry should capture, each Execution Unit should capture
    myROB.capture_results(b_vec);
    for(int i =0; i<b_vec.size(); i++){
        units[0].capture(b_vec[i].first,b_vec[i].second);
        units[1].capture(b_vec[i].first,b_vec[i].second);
        units[2].capture(b_vec[i].first,b_vec[i].second);
        units[3].capture(b_vec[i].first,b_vec[i].second);
        units[4].capture(b_vec[i].first,b_vec[i].second);
        units[5].capture(b_vec[i].first,b_vec[i].second);
    }
}

int Processor::getUnitIdx(OpCode op){
    if(op == OpCode::ADD || op == OpCode::ADD || op == OpCode::SUB)
    return 0;
    else if (op == OpCode::MUL)
    return 1;
    else if(op == OpCode::DIV || op == OpCode::REM)
    return 2;
    else if(op == OpCode::BEQ || op == OpCode::BNE || op == OpCode::BLT || op == OpCode::BLE || op == OpCode::J)
    return 3;
    else if(op == OpCode::SLT || op == OpCode::SLT || op == OpCode::AND || op == OpCode::ANDI || op == OpCode::OR || op == OpCode::ORI || op == OpCode::XOR || op == OpCode::XORI)
    return 4;
    else return 5;
}

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
    OpCode op = D_reg.inst.op;
    int uId = getUnitIdx(op);
    //cases acc to which instruction it is. BUt call step to all exe units regardless.
    if(myROB.is_Full() || units[uId].isRSFull()){/*add RS of desired exe unit full condition*/
        //stall
        return;
    }
    else{
        myROB.push(D_reg.inst);
        RSEntry temp_rs_entry;
        temp_rs_entry.ROB_Entry = myROB.youngest_entry;
        temp_rs_entry.op = D_reg.inst.op;
        temp_rs_entry.src1_valid = myRAT.reg_valid(D_reg.inst.src1);
        temp_rs_entry.src2_valid = myRAT.reg_valid(D_reg.inst.src2);
        if(temp_rs_entry.src1_valid && temp_rs_entry.src1_valid){
            temp_rs_entry.src1_value=ARF[D_reg.inst.src1];
            temp_rs_entry.src2_value=ARF[D_reg.inst.src2];
        }
        else if (temp_rs_entry.src1_valid){
            temp_rs_entry.src1_value=ARF[D_reg.inst.src1];
            temp_rs_entry.src2_tag=myRAT.get_alias(D_reg.inst.src2);
        }
        else if (temp_rs_entry.src2_valid){
            temp_rs_entry.src2_value=ARF[D_reg.inst.src2];
            temp_rs_entry.src1_tag=myRAT.get_alias(D_reg.inst.src1);
        }
        else{
            temp_rs_entry.src1_tag=myRAT.get_alias(D_reg.inst.src1);
            temp_rs_entry.src2_tag=myRAT.get_alias(D_reg.inst.src2);
        }
        
        //push to appropriate reservation station too.
        units[uId].pushToRS(temp_rs_entry);
    }
};

void Processor::stageExecuteAndBroadcast() {
    //all entries, if any are out. then they are written to the ROB and written to the RS by the broadcast.
    //it seems we do need a broadcast vector. Temporary one is made in every cycle.
    std::pair<int,int> temp;
    std::vector <std::pair<int,int>> broadcast_vector;
    for(int i=0;i<6;i++){
        temp = units[i].executeCycle();
        if(temp.first != -1){
        broadcast_vector.push_back(temp);
        units[i].loadToPipeline();
        }
    } //the broadcast vector now contains all the results of the calculations.
    //tell all execution units to get a new entry in the works too.
    broadcastOnCDB(broadcast_vector);
}

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