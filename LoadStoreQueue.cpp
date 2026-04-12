#include "LoadStoreQueue.h"

void LoadStoreQueue::push(RSEntry temp){
    if(!isFull()){
        youngest_entry=(youngest_entry+1)%size;
        RS_stage_vector[youngest_entry].valid=true;    
        RS_stage_vector[youngest_entry].rs_entry=temp;  
        RS_stage_vector[youngest_entry].stage=-1;//default we say not entered pipeline. If operands are ready, we will make something enter.    
    }
    return;    
}  

int LoadStoreQueue::get_valid_entry(){
    // go down from oldest to as long as stuff is valid
    int count = 0;
    int i = oldest_entry;
    while (count < pipeline_size) {
        if (!RS_stage_vector[i].valid) return -1;
        else if(RS_stage_vector[i].stage==-1){ // Othere field is immediate, it is always valid.
            if(RS_stage_vector[i].rs_entry.op==OpCode::LW && RS_stage_vector[i].rs_entry.src1_valid){
            return i;}
            else if(RS_stage_vector[i].rs_entry.op==OpCode::SW && RS_stage_vector[i].rs_entry.src1_valid && RS_stage_vector[i].rs_entry.src2_valid){ // Othere field is immediate, always valid.
            return i;}
            else{ return -1;} // an older entry itself is unready, beyond it nothing qualifies.
            }
        i = (i + 1) % size;
        count++;
    }
    return -1;
}

void LoadStoreQueue::reset() {
    RS::reset();
    youngest_entry = 0;
    oldest_entry = 1;
}

void LoadStoreQueue::invalidate_entry(int idx){
    // oldest_entry update too.
    if(idx == -1) return;
    oldest_entry = (oldest_entry+1)%size;
    RS_stage_vector[idx].valid = false;
    RS_stage_vector[idx].stage = -1;
}

void LoadStoreQueue::update_mem_addr_val(){
    for(int i=0;i<size;i++){
        if(RS_stage_vector[i].valid){
            if(RS_stage_vector[i].rs_entry.op == OpCode::LW){
                if(RS_stage_vector[i].rs_entry.src1_valid && !RS_stage_vector[i].rs_entry.mem_addr_valid    ){ //prevent multiple accesses to this by a mem addr_valid bit? or ok?
                    RS_stage_vector[i].rs_entry.mem_addr = RS_stage_vector[i].rs_entry.src1_value + RS_stage_vector[i].rs_entry.imm_value;
                    RS_stage_vector[i].rs_entry.mem_addr_valid = true;
                }   
            }
            else if(RS_stage_vector[i].rs_entry.op == OpCode::SW){
                if(RS_stage_vector[i].rs_entry.src2_valid && !RS_stage_vector[i].rs_entry.mem_addr_valid){
                    RS_stage_vector[i].rs_entry.mem_addr = RS_stage_vector[i].rs_entry.src2_value + RS_stage_vector[i].rs_entry.imm_value;
                    RS_stage_vector[i].rs_entry.mem_addr_valid = true;
                }
                if(RS_stage_vector[i].rs_entry.src1_valid && !RS_stage_vector[i].rs_entry.mem_val_valid){
                    RS_stage_vector[i].rs_entry.mem_val = RS_stage_vector[i].rs_entry.src1_value;
                    RS_stage_vector[i].rs_entry.mem_val_valid = true;
                }
            }
        }
    }
}

std::pair<int, int> LoadStoreQueue::latest_sw_idx(int lw_idx){ //returns <latest_sw_idx, mem_val> if both can be found.
    int latest_sw = -1;
    if(lw_idx==oldest_entry){
        return {-1,-1};
    }
    int i = (lw_idx-1+size)%size;
    while(true){
        if(RS_stage_vector[i].valid && RS_stage_vector[i].rs_entry.op == OpCode::SW){
            if(RS_stage_vector[i].rs_entry.mem_addr_valid && RS_stage_vector[lw_idx].rs_entry.mem_addr_valid){
                if(RS_stage_vector[i].rs_entry.mem_addr == RS_stage_vector[lw_idx].rs_entry.mem_addr){//validity of mem_addr and mem_val bt is NEEDED.
                    latest_sw = i;
                    if(RS_stage_vector[i].rs_entry.mem_val_valid){
                        return {latest_sw, RS_stage_vector[i].rs_entry.mem_val};
                    }
                    else{
                        // we have to wait for the sw to get its mem_val
                        return {-1,-1};
                    }
                }
            }
        }
        if(i==oldest_entry) break;
        i = (i-1+size)%size;

    }
    return {-1,-1};
}

void LoadStoreQueue::ls_fwd(){ 
    
    int i = oldest_entry;
    if(i==youngest_entry){
        return;
    }
    i=(i+1)%size;
    while(true){
        if(!RS_stage_vector[i].valid){
            break;
        }
        
        if(RS_stage_vector[i].rs_entry.op == OpCode::LW){
            if(RS_stage_vector[i].rs_entry.mem_addr_valid && !RS_stage_vector[i].rs_entry.ls_fwded && !RS_stage_vector[i].rs_entry.mem_val_valid){
                std::pair<int, int> latest_sw = latest_sw_idx(i);
                if(latest_sw.first != -1){
                    RS_stage_vector[i].rs_entry.ls_fwded = true;
                    RS_stage_vector[i].rs_entry.mem_val = latest_sw.second;
                    RS_stage_vector[i].rs_entry.mem_val_valid = true;
                }
            }
        }
        i = (i+1)%size;

        if(i==(youngest_entry+1)%size){
            break;
        }
    }
}