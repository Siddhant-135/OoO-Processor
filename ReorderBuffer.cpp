#include "ReorderBuffer.h"



void ROB::push(Instruction inst){
    if (is_Full()) return;
    else{
        youngest_entry_idx=(youngest_entry_idx+1)%capacity;
        ROB_Vector[youngest_entry_idx].valid = true;
        ROB_Vector[youngest_entry_idx].ready_from_RS = false;
        ROB_Vector[youngest_entry_idx].dest_regId = inst.dest;//lw, sw are not included in this. 
        ROB_Vector[youngest_entry_idx].dest_regVal = -1;//will be updated upon .
        ROB_branch_prediction[youngest_entry_idx] = BP_info{};
        ROB_Vector[youngest_entry_idx].dest_memAddr = -1;
        ROB_Vector[youngest_entry_idx].dest_memVal = 0;
        ROB_Vector[youngest_entry_idx].inst_pc = inst.pc;
        ROB_Vector[youngest_entry_idx].has_exception = false;
        number_of_entries++;
    }
}

bool ROB::pop(){ 
    if(is_Empty()){
        return false;
    }
    //also deallocate the RAT ki place.
    if(!ROB_Vector[oldest_entry_idx].ready_from_RS) return false; // if the entry is not ready, we cannot pop it. This is the only condition for not popping, because we want to commit in order.
    ROB_Vector[oldest_entry_idx].valid = false;
    ROB_branch_prediction[oldest_entry_idx] = BP_info{};
    oldest_entry_idx = (oldest_entry_idx+1)%capacity;
    number_of_entries--;
    return true;
}

//at every cycle
void ROB::rob_capture_results(std::vector <ExuResult> tags_values){

    for(int i=0;i<tags_values.size();i++){
        ROB_Vector[tags_values[i].tag].dest_regVal = tags_values[i].value;
        ROB_Vector[tags_values[i].tag].ready_from_RS = true;
        ROB_Vector[tags_values[i].tag].dest_memAddr = tags_values[i].mem_addr;
        ROB_Vector[tags_values[i].tag].dest_memVal = tags_values[i].mem_val;
        ROB_Vector[tags_values[i].tag].has_exception = tags_values[i].has_exception;
        std::cout<<"ROB capture: Updated ROB entry with tag "<<tags_values[i].tag<<" to value "<<tags_values[i].value<<"and mem addr "<<tags_values[i].mem_addr<<"and mem val "<<tags_values[i].mem_val<<"\n";
    }
    return;
}

void ROB::reset() {
    for (int i = 0; i < capacity; i++) {
        ROB_Vector[i].valid = false;
        ROB_Vector[i].ready_from_RS = false;
        ROB_Vector[i].dest_regId = -1;
        ROB_Vector[i].dest_regVal = 0;
        ROB_branch_prediction[i] = BP_info{};
        ROB_Vector[i].dest_memAddr = -1;
        ROB_Vector[i].dest_memVal = 0;
        ROB_Vector[i].inst_pc = -1;
        ROB_Vector[i].has_exception = false;
    }
    number_of_entries = 0;
    oldest_entry_idx = 1;
    youngest_entry_idx = 0;
}
