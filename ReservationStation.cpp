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

int RS::get_valid_entry(){
    for(int i=0;i<size;i++){
        if((RS_stage_vector[i].valid) && RS_stage_vector[i].stage==-1 && RS_stage_vector[i].rs_entry.src1_valid && RS_stage_vector[i].rs_entry.src2_valid){
            // RS_stage_vector[i].stage=0; NOT YET.
            return i;
        }
    }
    return -1;    
}

void RS::pushToPipeline(int idx){
    if(idx >= 0 && idx < size){
        RS_stage_vector[idx].stage=1;
    }
    return;    
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

void RS::reset() {
    for (int i = 0; i < size; i++) {
        RS_stage_vector[i].valid = false;
        RS_stage_vector[i].stage = -1;
    }
    counter = 0;
    PipelineCounter = 0;
}

void RS::invalidate_entry(int idx){
    RS_stage_vector[idx].valid=false;
    return;
}

void RS::rs_capture(int tag, int value){
    std::cout<<"    RS capture: Checking RS entries for tag "<<tag<<"\n";
    for(int i=0;i<size;i++){
        if(RS_stage_vector[i].valid){
            std::cout<<"    valid entry details:"<<"\n";
            std::cout<<"        src1_valid "<<RS_stage_vector[i].rs_entry.src1_valid<<" src1_value "<<RS_stage_vector[i].rs_entry.src1_value<<" src1_tag "<<RS_stage_vector[i].rs_entry.src1_tag<<"\n";
            if((!RS_stage_vector[i].rs_entry.src1_valid) && RS_stage_vector[i].rs_entry.src1_tag==tag){ // TO CHECK : ROB ENTRY OR SRC1 ENTRY ??
                RS_stage_vector[i].rs_entry.src1_valid=true;
                RS_stage_vector[i].rs_entry.src1_value=value;
                std::cout<<"        RS capture: Updated src1 of RS entry with ROB tag "<<tag<<" to value "<<value<<"\n";
            }
            std::cout<<"        src2_valid "<<RS_stage_vector[i].rs_entry.src2_valid<<" src2_value "<<RS_stage_vector[i].rs_entry.src2_value<<" src2_tag "<<RS_stage_vector[i].rs_entry.src2_tag<<"\n";
            if((!RS_stage_vector[i].rs_entry.src2_valid) && RS_stage_vector[i].rs_entry.src2_tag==tag){
                RS_stage_vector[i].rs_entry.src2_valid=true;
                RS_stage_vector[i].rs_entry.src2_value=value;
                std::cout<<"        RS capture: Updated src2 of RS entry with ROB tag "<<tag<<" to value "<<value<<"\n";
            }
        }
    }
}
