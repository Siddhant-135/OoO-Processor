#include "Processor.h"

// SETUP
Processor::Processor(ProcessorConfig& config) :myROB(config.rob_size), myRAT(config.num_regs){
    pc = 0;
    clock_cycle = 0;

    // Instantiate Hardware Units
    ARF.resize(config.num_regs, 0);
    Memory.resize(config.mem_size, 0);
    Parser myparser = Parser();

    // Keeping unit ordering stable for consistent index assumptions elsewhere:
    // ADDER, MULTIPLIER, DIVIDER, BRANCH, LOADSTORE, LOGIC
    units.push_back(ExecutionUnit(UnitType::ADDER, config.add_lat, config.adder_rs_size, Memory));
    units.push_back(ExecutionUnit(UnitType::MULTIPLIER, config.mul_lat, config.mult_rs_size, Memory));
    units.push_back(ExecutionUnit(UnitType::DIVIDER, config.div_lat, config.div_rs_size, Memory));
    units.push_back(ExecutionUnit(UnitType::BRANCH, config.add_lat, config.br_rs_size, Memory));
    units.push_back(ExecutionUnit(UnitType::LOADSTORE, config.mem_lat, config.lsq_rs_size, Memory));
    units.push_back(ExecutionUnit(UnitType::LOGIC, config.logic_lat, config.logic_rs_size, Memory));
}

void Processor::loadProgram(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {throw std::runtime_error("corrupted file");};
    myparser.parseFile(file, Processor::inst_memory, Processor::Memory);
}

void Processor::flush() {};

void Processor::broadcastOnCDB( std::vector<std::pair<int,int>> b_vec){
    //each ROB entry should , each Execution Unit should 
    myROB.rob_capture_results(b_vec);
    for(int j=0; j<units.size(); j++)
    {
        for(int i =0; i<b_vec.size(); i++){
            units[j].exu_capture (b_vec[i].first,b_vec[i].second);
        }
    }
}

int Processor::getUnitIdx(OpCode op){
    UnitType target = ExecutionUnit::getUnitTypeForOp(op);
    for (int i = 0; i < units.size(); i++) {
        if (units[i].name == target) {
            return i;
        }
    }
    return -1;
}

void Processor::stageFetch() {
    if (pc >= inst_memory.size()) {
        return;
    }
    F_reg.inst = inst_memory[pc];
    pc += 1;
}

void Processor::stageDecode() {
    //we think decode never stalls. // I support her thesis for now.
    D_reg.inst = F_reg.inst;
    OpCode op = D_reg.inst.op;
    int uId = getUnitIdx(op);
    if (uId == -1) {
        return;
    }
    //cases acc to which instruction it is. But call step to all exe units regardless.
    if(myROB.is_Full() || units[uId].isRSFull()){
        //stall
        return;
    }
    else{
        myROB.push(D_reg.inst);
        RSEntry temp_rs_entry;
        temp_rs_entry.ROB_Entry = myROB.newest_entry_idx();
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
    for(int i=0;i<units.size();i++){
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
    ROBEntry entry = myROB.to_be_commited_entry(); //Returns oldest entry.
    std::cout<<"Trying to commit ROB entry with dest reg "<<entry.dest_regId<<" and value "<<entry.dest_regVal<<"\n";
    int idx = entry.dest_regId;
    if(myROB.pop()){
        if (idx>=0) ARF[idx] = entry.dest_regVal; // Prevent initial crash because of -1 access etcetera.
        myRAT.rem_from_RAT(idx); // was crashing from out of bound access.
    }
};
    
bool Processor::step() {
    if (exception) {
        flush();
        return false;
    }
    if (pc >= inst_memory.size() + 20) { // HALTING CONDITION. bool like a flag to stop doing steps.
        return false;
    }
    stageCommit();
    stageExecuteAndBroadcast();
    stageDecode();
    stageFetch();
    clock_cycle++;
    return true;
}
