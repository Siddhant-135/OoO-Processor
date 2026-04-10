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

// TO DO: FORWARDING FOR LOAD AFTER STORE.
int LoadStoreQueue::get_valid_entry(){
    // go down from oldest to as long as stuff is valid
    int idx = -1;
    for(int i = oldest_entry; i < (oldest_entry+pipeline_size)%size; (i=i+1)%size){
        if(!RS_stage_vector[i].valid){
            return -1;
        }
        else if(RS_stage_vector[i].stage==-1 && RS_stage_vector[i].rs_entry.src1_valid){ // Othere fields are dest and immediate, they are always valid.
            return i;
        }
    }
    return -1;
}

void LoadStoreQueue::reset() {
    RS::reset();
    youngest_entry = 0;
    oldest_entry = 1;
}
