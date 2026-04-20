#include "Processor.h"

// SETUP
Processor::Processor(ProcessorConfig& config) :myROB(config.rob_size), myRAT(config.num_regs){
    pc = 0;
    clock_cycle = 0;
    verbose = config.verbose;

    // Instantiate Hardware Units
    ARF.resize(config.num_regs, 0);
    Memory.resize(config.mem_size, 0);

    // Keeping unit ordering stable for consistent index assumptions elsewhere:
    // ADDER, MULTIPLIER, DIVIDER, BRANCH, LOADSTORE, LOGIC
    units.push_back(ExecutionUnit(UnitType::ADDER, config.add_lat, config.adder_rs_size, Memory, verbose));
    units.push_back(ExecutionUnit(UnitType::MULTIPLIER, config.mul_lat, config.mult_rs_size, Memory, verbose));
    units.push_back(ExecutionUnit(UnitType::DIVIDER, config.div_lat, config.div_rs_size, Memory, verbose));
    units.push_back(ExecutionUnit(UnitType::BRANCH, config.add_lat, config.br_rs_size, Memory, verbose));
    units.push_back(ExecutionUnit(UnitType::LOADSTORE, config.mem_lat, config.lsq_rs_size, Memory, verbose));
    units.push_back(ExecutionUnit(UnitType::LOGIC, config.logic_lat, config.logic_rs_size, Memory, verbose));
    rs_full_before_execute.resize(units.size(), false);
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

    for (size_t i = 0; i < units.size(); i++) {
        units[i].reset();
    }
}

void Processor::broadcastOnCDB( std::vector<ExuResult> b_vec){
    //each ROB entry should , each Execution Unit should 
    if (verbose) std::cout<<"In Broadcast\n";
    myROB.rob_capture_results(b_vec);
    for(size_t j=0; j<units.size(); j++)
    {
        for(size_t i =0; i<b_vec.size(); i++){
            units[j].exu_capture (b_vec[i].tag,b_vec[i].value); // units dont have to capture the result of a sw
        }
    }
}

int Processor::getUnitIdx(OpCode op){
    UnitType target = ExecutionUnit::getUnitTypeForOp(op);
    for (size_t i = 0; i < units.size(); i++) {
        if (units[i].name == target) {
            return i;
        }
    }
    return -1;
}

void Processor::stageFetch() {
    if (static_cast<size_t>(pc) >= inst_memory.size()) { // F_reg.valid check is for the case when fetch is stalled so we dont want to overwrite the fetched instruction.
        if (verbose) std::cout<<pc<<"PC has exceeded instruction memory. "<<inst_memory.size()<< " No more instructions to fetch.\n";
        return;
    }
    if( F_reg.valid){
        if (verbose) std::cout<<pc<<"Fetch register is already full. Stall: due to full ROB or RS or LSQ\n";
        return;
    }
    int current_pc = pc;
    F_reg.inst = inst_memory[current_pc]; 
    F_reg.bp_info = bp.make_prediction_info(current_pc, F_reg.inst.imm, F_reg.inst.op);
    if (verbose) std::cout<<"pc: "<<current_pc<<" fetched instruction with opcode "<<static_cast<int>(F_reg.inst.op)<<"\n";
    F_reg.valid = true;
    pc = F_reg.bp_info.predicted_next_pc; // complete takeover by bp, pc+=1 bhi isme hi hai, tbf early on bhi conditional daal sakte but works for now.
    if (verbose) std::cout<<"Branch Predictor predicted next pc to be "<<F_reg.bp_info.predicted_next_pc<<"\n";
}

void Processor::stageDecode() {
    //we think decode never stalls. // I support her thesis for now. Decode will stall if RS or ROB or LSQ is full
    //need control for the 1st instruction: 
    //valid bit in F_reg, D_reg
    if(!F_reg.valid && !D_reg.valid){ // geenral cases would have been handled by the fetch call but if pc reached end of program and then a stall happened, the decode would never retry.
        if (verbose) std::cout<<"No valid instruction in fetch register to decode or nothing in decode register.\n";
        return;
    }
    // if D_reg valid is true, don't replace the old entry, rather try again.
    
    if(!D_reg.valid){ //D_reg is ready to take the new entry and free the valid F_Reg
        D_reg.inst = F_reg.inst;
        D_reg.bp_info = F_reg.bp_info;
        F_reg.valid = false; // remains false until next fetch. GOOD THING, KEEP. 
        D_reg.valid = true;
        OpCode op = D_reg.inst.op;
        int uId = getUnitIdx(op);
        if (verbose) std::cout<<"Decoded instruction with opcode "<<static_cast<int>(D_reg.inst.op)<<" for execution unit "<<uId<<"\n";
        if (uId == -1) {
            if (verbose) std::cout<<"Unsupported opcode encountered during decode. Halting.\n";
            return;
        }

        // J (unconditional jump) is fully resolved at fetch — BP already set pc = target.
        // No register write, no memory effect, no need for ROB/RS/execute/commit.
        // Just consume it and free the decode register.
        // if (D_reg.inst.op == OpCode::J) {
        //     if (verbose) std::cout<<"Jump instruction resolved at fetch. Consuming at decode, no ROB/RS needed.\n";
        //     D_reg.valid = false;
        //     return;
        // }
    }

    // THE ELSE PART, I.E A STILL-VALID D_REG, MEANING A DECODE IN ITS STALL STATE. ALSO HAS TO EECUTE FOR A NEWLY ENCOUNTERED DECODE SO CHILL
    //cases acc to which instruction it is. But call step to all exe units regardless.
    int uId = getUnitIdx(D_reg.inst.op);
    if(myROB.is_Full() || rs_full_before_execute[static_cast<size_t>(uId)]){ // RS fullness from before execute (same cycle).
        //stall
        if (verbose) {
            if(myROB.is_Full()) std::cout<<"Stalling at decode stage due to full ROB.\n";
            else std::cout<<"Stalling at decode stage due to full RS of execution unit "<<uId<<".\n";
            std::cout<<"Stalling at decode stage due to full ROB or RS.\n";
        }
        return;
    }
    else{
        if (verbose) std::cout<<"Pushing instruction to ROB and RS for execution unit "<<uId<<"\n";
        myROB.push(D_reg.inst);
        // Make the D_reg.inst.destRegId as false too!! happens in My_RAT.add_to RAT.
        int rob_tag = myROB.newest_entry_idx();
        myROB.set_branch_prediction(rob_tag, D_reg.bp_info);
        RSEntry temp_rs_entry;
        temp_rs_entry.ROB_Entry = rob_tag;
        temp_rs_entry.dispatch_seq = dispatch_counter++;
        temp_rs_entry.op = D_reg.inst.op;
    
        if (D_reg.inst.src1 < 0) temp_rs_entry.src1_valid = true; // is this causing error for the BEQ case? Should not be so
        else temp_rs_entry.src1_valid = myRAT.reg_valid(D_reg.inst.src1);

        if (D_reg.inst.src2 < 0) temp_rs_entry.src2_valid = true;
        else temp_rs_entry.src2_valid = 
        myRAT.reg_valid(D_reg.inst.src2);

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

        if (verbose) {
            std::cout<<"RS entry details: src1_valid "<<temp_rs_entry.src1_valid<<" src1_value "<<temp_rs_entry.src1_value<<" src1_tag "<<temp_rs_entry.src1_tag<<"\n";
            std::cout<<"RS entry details: src2_valid "<<temp_rs_entry.src2_valid<<" src2_value "<<temp_rs_entry.src2_value<<" src2_tag "<<temp_rs_entry.src2_tag<<"\n";
        }
        temp_rs_entry.imm_value = D_reg.inst.imm;
        temp_rs_entry.dest_value = D_reg.inst.dest;
        if (D_reg.inst.dest > 0) { // sw doesnt have a dest.
            myRAT.add_to_RAT(D_reg.inst.dest, temp_rs_entry.ROB_Entry);
        }
        //push to appropriate reservation station too.
        if (verbose) std::cout<<"Pushing to RS of execution unit "<<uId<<" with ROB tag "<<temp_rs_entry.ROB_Entry<<"\n";
        units[uId].pushToRS(temp_rs_entry);
        
        // Reset decode register valid flag meaning we've fully dispatched this instruction
        D_reg.valid = false;
    }
};

void Processor::stageExecuteAndBroadcast() {
    //all entries, if any are out. then they are written to the ROB and written to the RS by the broadcast.
    //it seems we do need a broadcast vector. Temporary one is made in every cycle.
    ExuResult temp;
    std::vector <ExuResult> broadcast_vector;
    for(size_t i=0;i<units.size();i++){
        if (verbose) std::cout<<"Executing unit "<<i<<"\n";
        temp = units[i].executeCycle();
        if(temp.tag != -1){
            if (verbose) {
                std::cout<<"    Execution result: tag "<<temp.tag<<" value "<<temp.value<<"\n";
                std::cout<<"    Adding to broadcast vector: tag "<<temp.tag<<" value "<<temp.value<<"\n";
            }
            broadcast_vector.push_back(temp);
            // units[i].loadToPipeline(); // REDUNDANT & BUGGY: executeCycle() already handles capacity bounds
        }
    } //the broadcast vector now contains all the results of the calculations.
    //tell all execution units to get a new entry in the works too.
    if (verbose) std::cout<<"Broadcasting results to CDB.\n";
    broadcastOnCDB(broadcast_vector);
}

void Processor::stageCommit() {
    if (myROB.is_Empty()) {
        if (verbose) std::cout<<"Cannot commit yet. ROB is empty \n";
        return;
    }
    int oldest_tag = myROB.oldest_idx();
    ROBEntry entry = myROB.to_be_commited_entry(); //Returns oldest entry., and the ready one. In BP context it is the real entry.
    BP_info pred_info = myROB.to_be_commited_prediction(); // The prediction by the Branch Predictor. If the entry isnt a branch, the validity bit of this is zero.
    if (verbose) std::cout<<"Trying to commit ROB entry with dest reg "<<entry.dest_regId<<" and value "<<entry.dest_regVal<<"\n";
    int idx = entry.dest_regId; // -1 for lot of commands, such as SW, Branches etc.

    if(entry.ready_from_RS){
    if (entry.has_exception) {
        if (verbose) std::cout << "Exception at commit for ROB entry " << oldest_tag
                              << " (instruction PC " << entry.inst_pc << "). Halting.\n";
        pc = entry.inst_pc;
        exception = true;
        flush();
        return;
    }
    if (pred_info.valid && pred_info.is_conditional) { // check for flush etc only for conditional branches.
        int actual_next_pc = pred_info.fallthrough_pc;
        bool taken = (entry.dest_regVal != 0); // branch fails if the int version of the boolean operation is 0, taken if NOT.
        if (taken) actual_next_pc = pred_info.target_pc;
        else actual_next_pc = pred_info.fallthrough_pc; //repetitive, but we learn by repetition
        bool was_correct = (actual_next_pc == pred_info.predicted_next_pc);
        bp.update(pred_info.fetch_pc, taken, was_correct); //update the FSM.

        if (actual_next_pc != pred_info.predicted_next_pc) {
            if (verbose) std::cout<<"Branch misprediction at PC "<<pred_info.fetch_pc<<". Flushing younger instructions.\n";
            pc = actual_next_pc;
            myROB.pop(); // THE ENTRY IS READY SO POP IS WORKING.
            flush();
            flushed_this_cycle = true;
            return;
        }
        else{
            if (verbose) std::cout<<"Branch prediction correct for branch at PC "<<pred_info.fetch_pc<<".\n";
            myROB.pop(); // THE ENTRY IS READY SO POP IS WORKING.
            return;
        }
    }

    if(myROB.pop()){
        if (idx > 0) { // Protects against -1 (Branches/SW) and 0 (x0 is hardwired to 0)
            ARF[idx] = entry.dest_regVal;
            // RAT CLEAR BUG: Only clear RAT if it still points to the commiting instruction
            if (myRAT.get_alias(idx) == oldest_tag) {
                myRAT.rem_from_RAT(idx); 
            }
            if (verbose) std::cout<<"Committed ROB entry with dest reg "<<idx<<" and value "<<entry.dest_regVal<<"\n";
        }
        else{
            if (entry.dest_memAddr != -1) {
                Memory[entry.dest_memAddr] = entry.dest_memVal;
                if (verbose) std::cout<<"Committed SW instruction at address "<<entry.dest_memAddr<<" with value "<<entry.dest_memVal<<"\n";
            }
            else if (verbose) std::cout<<"Removed from ROB the entry trying to commit to x0 and value: "<<entry.dest_regVal<<"\n";
        }
    }
    // REDUNDANT/BUGGY LOGIC REMOVED FROM HERE
    }
    else{
        if (verbose) std::cout<<"Cannot commit yet. The oldest value isn't ready.\n";
        }
};
    
bool Processor::step() {
    flushed_this_cycle = false;
    if (exception) {
        return false;
    }
    
    // PERFECT HALTING CONDITION:
    // 1. No more instructions in memory to fetch (pc >= inst_memory.size())
    // 2. Fetch register has transferred its contents (!F_reg.valid)
    // 3. Decode register has dispatched its contents (!D_reg.valid)
    // 4. All dispatched instructions have committed (myROB.is_Empty())
    if(static_cast<size_t>(pc) >= inst_memory.size() && !F_reg.valid && !D_reg.valid && myROB.is_Empty()){ 
        if (verbose) std::cout<<"\n[+] Execution complete! Total clock cycles: "<<clock_cycle<<"\n";
        return false;
    }

    if (verbose) {
        std::cout << "\n--- CYCLE " << clock_cycle << " (PC: " << pc << ") ---\n";
        myRAT.print();
        myROB.print();
        for (const auto& unit : units) {
            unit.print();
        }
    }

    stageCommit();
    if (exception) {
        clock_cycle += 1;
        return false;
    }
    if (flushed_this_cycle) {
        clock_cycle++;
        return true;
    }
    for (size_t i = 0; i < units.size(); i++) {
        rs_full_before_execute[i] = units[i].isRSFull();
    }
    stageExecuteAndBroadcast();
    stageDecode();
    stageFetch();
    clock_cycle++;
    return true;
}
