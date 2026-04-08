#pragma once
#include "ReservationStation.h"

bool RS::isFull(){
    for(int i=0;i<size;i++){
        if(!RS_stage_vector[i].first){
            return false;
        }
    }
    return true;
};

void RS::push(RSEntry temp){ //create the RS vector at the time of decode itself from the Processor and RAT.
    for(int i=0;i<size;i++){
        if(!RS_stage_vector[i].first){
            RS_stage_vector[i].first=true;    
            RS_stage_vector[i].second.first=temp;  
            RS_stage_vector[i].second.second=-1;//default we say not entered pipeline. If operands are ready, we will make something enter.
            return;
        }
    }
    return;    
}

int RS::get_valid_entry(){//modify its pipeline stage entry to 0
    for(int i=0;i<size;i++){
        if((RS_stage_vector[i].first) && RS_stage_vector[i].second.second==-1 && RS_stage_vector[i].second.first.src1_valid && RS_stage_vector[i].second.first.src2_valid){
            RS_stage_vector[i].second.second=0;
            return i;
        }
    }
    return -1;    
}

int RS::step_rs_get_final(){//does op matter here? No, the parent, execution, takes care of it. modifying this
    int idx = -1;
    counter++;
    if(counter == stage_lat){
        for(int i=0;i<size;i++){
            if(RS_stage_vector[i].first){
                if(RS_stage_vector[i].second.second>=0){
                    RS_stage_vector[i].second.second++;
                    if (RS_stage_vector[i].second.second==pipeline_size){
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
    RS_stage_vector[idx].first=false;
    return;
}

void RS::capture(int tag, int value){
    for(int i=0;i<size;i++){
        if(RS_stage_vector[i].first){
            if(!RS_stage_vector[i].second.first.src1_valid && RS_stage_vector[i].second.first.ROB_Entry==tag){
                RS_stage_vector[i].second.first.src1_valid=true;
                RS_stage_vector[i].second.first.src1_value=value;
            }
            if(!RS_stage_vector[i].second.first.src2_valid && RS_stage_vector[i].second.first.ROB_Entry==tag){
                RS_stage_vector[i].second.first.src2_valid=true;
                RS_stage_vector[i].second.first.src2_value=value;
            }
        }
    }
}