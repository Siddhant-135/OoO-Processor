#include "ReorderBuffer.h"

void ROB::push(Instruction inst){
//at end of every decode, the instruction gets pushed to queue of the ROB
if (is_Full){
    return;
    //ROB is full, no can push.
}
else{
    youngest_entry=(youngest_entry+1)%size;
    ROB_Vector[youngest_entry].valid = true;
    ROB_Vector[youngest_entry].ready_from_RS = false;
    ROB_Vector[youngest_entry].dest_regId = inst.dest;//either a destination in the rat or a registerID. 
    // ROB_Vector[youngest_entry+1].dest_regVal = ?
    // ROB_Vector[youngest_entry].src1 = inst.src1;//make it a triplet, bool, value, tag.
    // ROB_Vector[youngest_entry].src2 = inst.src2;
    // ROB_Vector[youngest_entry].sent_to_RS = false;
}
}

void ROB::pop(){//pop and write to the approriate register. Should be used in the commit (W) stage tho
    ROB_Vector[oldest_entry].valid = false;
    oldest_entry = (oldest_entry+1)%size;
    if(oldest_entry==youngest_entry){
        youngest_entry=0;
        oldest_entry=0;
    }
}
// void ROB::dispatch(){//push a command to RS, all possible that can be pushed get pushed.
// //internally set all the RSEntry values of the commands to be dispatched.
// }

void capture_results(std::vector <std::pair<int, int>> tags_values){

}