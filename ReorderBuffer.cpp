#include "ReorderBuffer.h"
#include <algorithm>


void ROB::push(Instruction inst){
//at end of every decode, the instruction gets pushed to queue of the ROB
if (is_Full()){
    return;
    //ROB is full, no can push.
}
else{
    youngest_entry=(youngest_entry+1)%size;
    ROB_Vector[youngest_entry].valid = true;
    ROB_Vector[youngest_entry].ready_from_RS = false;
    ROB_Vector[youngest_entry].dest_regId = inst.dest;//lw, sw are not included in this. 
    ROB_Vector[youngest_entry].dest_regVal = -1;//will be updated upon capture.
}
}

//call the change ARF function in the main processor itself.
//pop and write to the approriate register. Should be used in the commit (W) stage tho

//export the topmost ROB_Vector ka destReg

int ROB::dest_reg(){
    return ROB_Vector[oldest_entry].dest_regId;
}

int ROB::dest_val(){
    return ROB_Vector[oldest_entry].dest_regVal;
}

void ROB::pop(){ 
    if(oldest_entry==youngest_entry){//single element queue
        //also deallocate the RAT ki place.
        ROB_Vector[oldest_entry].valid = false;
        youngest_entry = 0;
        oldest_entry = 0;
        return;
    }
    ROB_Vector[oldest_entry].valid = false;
    oldest_entry = (oldest_entry+1)%size;
}

//at every cycle
void ROB::capture_results(std::vector <std::pair<int, int>> tags_values){

    for(int i=0;i<tags_values.size();i++){
        ROB_Vector[tags_values[i].first].dest_regVal = tags_values[i].second;
        ROB_Vector[tags_values[i].first].ready_from_RS = true;
    }
    return;
}