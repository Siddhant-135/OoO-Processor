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

// TO DO: FORWARDING FOR LOAD AFTER STORE.//THIS FUNCTION IS INCORRECT, NEEDS TO SEE THE LOAD AFTER STORE AD STORE AFTER LOAD CONFLICT AND THEN GET THE VALID ENTRY.
int LoadStoreQueue::get_valid_entry(){
    // go down from oldest to as long as stuff is valid
    int idx = -1;
    for(int i = oldest_entry; i < (oldest_entry+pipeline_size)%size; (i=i+1)%size){ //only see for entries that can be accomodated to the pipeline
        if(!RS_stage_vector[i].valid){//if older entry itself is inavlid and oldest hasnt been updated only then can this happen.
            return -1;
        }
        else if(RS_stage_vector[i].stage==-1){ // Othere field is immediate, it is always valid.
            if(RS_stage_vector[i].rs_entry.op==OpCode::LW && RS_stage_vector[i].rs_entry.src1_valid){
            return i;}
            else if(RS_stage_vector[i].rs_entry.op==OpCode::SW && RS_stage_vector[i].rs_entry.src1_valid && RS_stage_vector[i].rs_entry.src2_valid){ // Othere field is immediate, always valid.
            return i;}
            else{ return -1;} // an older entry itself is unready, beyond it nothing qualifies.
        }
        // else we check for the next rs entry.
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
