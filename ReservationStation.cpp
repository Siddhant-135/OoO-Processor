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