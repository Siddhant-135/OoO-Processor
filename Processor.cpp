#include "Processor.h"

// SETUP
Processor::Processor(ProcessorConfig& config) :myROB(config.rob_size), myRAT(config.num_regs){
    pc = 0;
    clock_cycle = 0;

    // Instantiate Hardware Units
    ARF.resize(config.num_regs, 0);
    Memory.resize(config.mem_size, 0);

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

void Processor::flush() {
    F_reg = Pipeline_reg{};
    D_reg = Pipeline_reg{};

    myROB.reset();
    myRAT.reset();

    for (int i = 0; i < units.size(); i++) {
        units[i].reset();
    }
}

void Processor::broadcastOnCDB( std::vector<std::pair<int,int>> b_vec){
    //each ROB entry should , each Execution Unit should 
    std::cout<<"In Broadcast\n";
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
    if (pc >= inst_memory.size() || F_reg.valid) { // F_reg.valid check is for the case when fetch is stalled so we dont want to overwrite the fetched instruction.
        std::cout<<pc<<"PC has exceeded instruction memory. "<<inst_memory.size()<< " No more instructions to fetch.\n";
        return;
    }
    int current_pc = pc;
    F_reg.inst = inst_memory[current_pc]; 
    F_reg.bp_info = bp.make_prediction_info(current_pc, F_reg.inst.imm, F_reg.inst.op);
    std::cout<<"pc: "<<current_pc<<" fetched instruction with opcode "<<static_cast<int>(F_reg.inst.op)<<"\n"; //HOW TO MAKE THE OPCODE STRING
    F_reg.valid = true;
    pc = F_reg.bp_info.predicted_next_pc; // complete takeover by bp, pc+=1 bhi isme hi hai, tbf early on bhi conditional daal sakte but works for now.
}

void Processor::stageDecode() {
    //we think decode never stalls. // I support her thesis for now.
    //need control for the 1st instruction: 
    //valid bit in F_reg, D_reg
    if(!F_reg.valid){
        std::cout<<"No valid instruction in fetch register to decode.\n";
        return;
    }
    D_reg.inst = F_reg.inst;
    D_reg.bp_info = F_reg.bp_info;
    F_reg.valid = false; // remains false until next fetch.
    D_reg.valid = true;
    OpCode op = D_reg.inst.op;
    int uId = getUnitIdx(op);
    std::cout<<"Decoded instruction with opcode "<<static_cast<int>(D_reg.inst.op)<<" for execution unit "<<uId<<"\n";
    if (uId == -1) {
        std::cout<<"Unsupported opcode encountered during decode. Halting.\n";
        return;
    }
    //cases acc to which instruction it is. But call step to all exe units regardless.
    if(myROB.is_Full() || units[uId].isRSFull()){
        //stall
        if(myROB.is_Full()) std::cout<<"Stalling at decode stage due to full ROB.\n";
        else std::cout<<"Stalling at decode stage due to full RS of execution unit "<<uId<<".\n";
        std::cout<<"Stalling at decode stage due to full ROB or RS.\n";
        return;
    }
    else{
        std::cout<<"Pushing instruction to ROB and RS for execution unit "<<uId<<"\n";
        myROB.push(D_reg.inst);
        int rob_tag = myROB.newest_entry_idx();
        myROB.set_branch_prediction(rob_tag, D_reg.bp_info);
        RSEntry temp_rs_entry;
        temp_rs_entry.ROB_Entry = rob_tag;
        temp_rs_entry.op = D_reg.inst.op;

        if (D_reg.inst.src1 < 0) temp_rs_entry.src1_valid = true;
        else temp_rs_entry.src1_valid = myRAT.reg_valid(D_reg.inst.src1);

        if (D_reg.inst.src2 < 0) temp_rs_entry.src2_valid = true;
        else temp_rs_entry.src2_valid = myRAT.reg_valid(D_reg.inst.src2);

        if(temp_rs_entry.src1_valid && temp_rs_entry.src2_valid){
            if (D_reg.inst.src1 >= 0) temp_rs_entry.src1_value=ARF[D_reg.inst.src1];
            if (D_reg.inst.src2 >= 0) temp_rs_entry.src2_value=ARF[D_reg.inst.src2];
        }
        else if (temp_rs_entry.src1_valid){
            temp_rs_entry.src1_value=ARF[D_reg.inst.src1];
            temp_rs_entry.src2_tag=myRAT.get_alias(D_reg.inst.src2);// Let there be a tag as well as a value: ready from RS is true then take the value and make src1 valid.
            if(myROB.get_ReadyFromRS(temp_rs_entry.src2_tag)){
                temp_rs_entry.src2_value=myROB.get_DestRegVal(temp_rs_entry.src2_tag);
                temp_rs_entry.src2_valid=true;
                temp_rs_entry.src2_tag=-1;
            }
        }
        else if (temp_rs_entry.src2_valid){
            temp_rs_entry.src2_value=ARF[D_reg.inst.src2];
            temp_rs_entry.src1_tag=myRAT.get_alias(D_reg.inst.src1);
            if(myROB.get_ReadyFromRS(temp_rs_entry.src1_tag)){
                temp_rs_entry.src1_value=myROB.get_DestRegVal(temp_rs_entry.src1_tag);
                temp_rs_entry.src1_valid=true;
                temp_rs_entry.src1_tag=-1;
            }
        }
        else{
            temp_rs_entry.src1_tag=myRAT.get_alias(D_reg.inst.src1);
            if(myROB.get_ReadyFromRS(temp_rs_entry.src1_tag)){
                temp_rs_entry.src1_value=myROB.get_DestRegVal(temp_rs_entry.src1_tag);
                temp_rs_entry.src1_valid=true;
                temp_rs_entry.src1_tag=-1;
            }
            temp_rs_entry.src2_tag=myRAT.get_alias(D_reg.inst.src2);
            if(myROB.get_ReadyFromRS(temp_rs_entry.src2_tag)){
                temp_rs_entry.src2_value=myROB.get_DestRegVal(temp_rs_entry.src2_tag);
                temp_rs_entry.src2_valid=true;
                temp_rs_entry.src2_tag=-1;
            }
        }

        std::cout<<"RS entry details: src1_valid "<<temp_rs_entry.src1_valid<<" src1_value "<<temp_rs_entry.src1_value<<" src1_tag "<<temp_rs_entry.src1_tag<<"\n";
        std::cout<<"RS entry details: src2_valid "<<temp_rs_entry.src2_valid<<" src2_value "<<temp_rs_entry.src2_value<<" src2_tag "<<temp_rs_entry.src2_tag<<"\n";
        temp_rs_entry.imm_value = D_reg.inst.imm;
        temp_rs_entry.dest_value = D_reg.inst.dest;
        if (D_reg.inst.dest > 0) {
            myRAT.add_to_RAT(D_reg.inst.dest, temp_rs_entry.ROB_Entry);
        }
        //push to appropriate reservation station too.
        std::cout<<"Pushing to RS of execution unit "<<uId<<" with ROB tag "<<temp_rs_entry.ROB_Entry<<"\n";
        units[uId].pushToRS(temp_rs_entry);
    }
};

void Processor::stageExecuteAndBroadcast() {
    //all entries, if any are out. then they are written to the ROB and written to the RS by the broadcast.
    //it seems we do need a broadcast vector. Temporary one is made in every cycle.
    std::pair<int,int> temp;
    std::vector <std::pair<int,int>> broadcast_vector;
    for(int i=0;i<units.size();i++){
        std::cout<<"Executing unit "<<i<<"\n";
        temp = units[i].executeCycle();
        std::cout<<"Execution result: tag "<<temp.first<<" value "<<temp.second<<"\n";
        if(temp.first != -1){
        std::cout<<"Adding to broadcast vector: tag "<<temp.first<<" value "<<temp.second<<"\n";
        broadcast_vector.push_back(temp);
        units[i].loadToPipeline();
        }
    } //the broadcast vector now contains all the results of the calculations.
    //tell all execution units to get a new entry in the works too.
    std::cout<<"Broadcasting results to CDB.\n";
    broadcastOnCDB(broadcast_vector);
}

void Processor::stageCommit() {
    if (myROB.is_Empty()) {
        std::cout<<"Cannot commit yet. ROB is empty \n";
        return;
    }
    int oldest_tag = myROB.oldest_idx();
    ROBEntry entry = myROB.to_be_commited_entry(); //Returns oldest entry.
    BP_info pred_info = myROB.to_be_commited_prediction();
    std::cout<<"Trying to commit ROB entry with dest reg "<<entry.dest_regId<<" and value "<<entry.dest_regVal<<"\n";
    int idx = entry.dest_regId;

    if (pred_info.valid && pred_info.is_conditional) { // check for flush etc only for conditional branches.
        int actual_next_pc = pred_info.fallthrough_pc;
        bool taken = (entry.dest_regVal != 0);
        if (taken) actual_next_pc = pred_info.target_pc;
        else actual_next_pc = pred_info.fallthrough_pc;
        bool was_correct = (actual_next_pc == pred_info.predicted_next_pc);
        bp.update(pred_info.fetch_pc, taken, was_correct);

        if (actual_next_pc != pred_info.predicted_next_pc) {
            std::cout<<"Branch misprediction at PC "<<pred_info.fetch_pc<<". Flushing younger instructions.\n";
            pc = actual_next_pc;
            flush();
            flushed_this_cycle = true;
            return;
        }
    }

    if(myROB.pop()){
        if (idx>=0) ARF[idx] = entry.dest_regVal; // Prevent initial crash because of -1 access etcetera.
        myRAT.rem_from_RAT(idx); // was crashing from out of bound access.
        std::cout<<"Committed ROB entry with dest reg "<<idx<<" and value "<<entry.dest_regVal<<"\n";

    }

    if (idx > 0) {
        ARF[idx] = entry.dest_regVal; // Prevent initial crash because of -1 access etcetera.
        if (myRAT.get_alias(idx) == oldest_tag) {
            myRAT.rem_from_RAT(idx); // was crashing from out of bound access.
            myROB.pop();
        }
    }
    else{
        std::cout<<"Cannot commit yet. Either ROB is empty or the oldest entry is not ready.\n";}
};
    
bool Processor::step() {
    flushed_this_cycle = false;
    if (exception) {
        flush();
        return false;
    }
    if(myROB.is_Empty() && pc >= inst_memory.size() && clock_cycle > 1){ 
        std::cout<<"clock cycle: "<<clock_cycle<<" No more instructions to commit and no more instructions to fetch. Halting.\n";
        return false;
    }
    // if (pc >= inst_memory.size() + 8) { // HALTING CONDITION. bool like a flag to stop doing steps.
    //     return false;
    // }
    std::cout<<"pc: "<<pc<<" clock cycle: "<<clock_cycle<<"\n";
    stageCommit();
    if (flushed_this_cycle) {
        clock_cycle++;
        return true;
    }
    stageExecuteAndBroadcast();
    stageDecode();
    stageFetch();
    clock_cycle++;
    return true;
}
