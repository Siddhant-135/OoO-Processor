#include "ReorderBuffer.h"
#include <algorithm>


void ROB::push(Instruction inst){
    if (is_Full()) return;
    else{
        youngest_entry_idx=(youngest_entry_idx+1)%size;
        ROB_Vector[youngest_entry_idx].valid = true;
        ROB_Vector[youngest_entry_idx].ready_from_RS = false;
        ROB_Vector[youngest_entry_idx].dest_regId = inst.dest;//lw, sw are not included in this. 
        ROB_Vector[youngest_entry_idx].dest_regVal = -1;//will be updated upon capture.
    }
}

void ROB::pop(){ 
    if(oldest_entry_idx==youngest_entry_idx){//single element queue
        //also deallocate the RAT ki place.
        ROB_Vector[oldest_entry_idx].valid = false;
        youngest_entry_idx = 0;
        oldest_entry_idx = 0;
        return;
    }
    ROB_Vector[oldest_entry_idx].valid = false;
    oldest_entry_idx = (oldest_entry_idx+1)%size;
}

//at every cycle
void ROB::capture_results(std::vector <std::pair<int, int>> tags_values){

    for(int i=0;i<tags_values.size();i++){
        ROB_Vector[tags_values[i].first].dest_regVal = tags_values[i].second;
        ROB_Vector[tags_values[i].first].ready_from_RS = true;
    }
    return;
}