#pragma once
#include "ReservationStation.h"

bool RS::isFull(){
    for(int i=0;i<size;i++){
        if(!RS_stage_vector[i].valid){
            return false;
        }
    }
    return true;
};

void RS::push(RSEntry temp){ //create the RS vector at the time of decode itself from the Processor and RAT.
    for(int i=0;i<size;i++){
        if(!RS_stage_vector[i].valid){
            RS_stage_vector[i].valid=true;    
            RS_stage_vector[i].rs_entry=temp;  
            RS_stage_vector[i].stage=-1;//default we say not entered pipeline. If operands are ready, we will make something enter.
            return;
        }
    }
    return;    
}

int RS::get_valid_entry(){//modify its pipeline stage entry to 0
    for(int i=0;i<size;i++){
        if((RS_stage_vector[i].valid) && RS_stage_vector[i].stage==-1 && RS_stage_vector[i].rs_entry.src1_valid && RS_stage_vector[i].rs_entry.src2_valid){
            RS_stage_vector[i].stage=0;
            return i;
        }
    }
    return -1;    
}

int RS::update_rs(){//does op matter here? No, the parent, execution, takes care of it. modifying this
    int idx = -1;
    counter++;
    if(counter == stage_lat){
        for(int i=0;i<size;i++){
            if(RS_stage_vector[i].valid){
                if(RS_stage_vector[i].stage>=0){
                    RS_stage_vector[i].stage++;
                    if (RS_stage_vector[i].stage==pipeline_size){
                    idx=i;
                    }
                }
            }
        }
        counter =0;
    }
    return idx;
}

void RS::invalidate_entry(int idx){
    RS_stage_vector[idx].valid=false;
    return;
}

void RS::capture(int tag, int value){
    for(int i=0;i<size;i++){
        if(RS_stage_vector[i].valid){
            if(!RS_stage_vector[i].rs_entry.src1_valid && RS_stage_vector[i].rs_entry.ROB_Entry==tag){
                RS_stage_vector[i].rs_entry.src1_valid=true;
                RS_stage_vector[i].rs_entry.src1_value=value;
            }
            if(!RS_stage_vector[i].rs_entry.src2_valid && RS_stage_vector[i].rs_entry.ROB_Entry==tag){
                RS_stage_vector[i].rs_entry.src2_valid=true;
                RS_stage_vector[i].rs_entry.src2_value=value;
            }
        }
    }
}