#pragma once
#include "Basics.h"
#include <vector>
#include <optional>

class RS {
    public:
    //constructor
    RS(int size, int pipeline_size, int stage_lat){
        this->size=size;
        RS_stage_vector.resize(size);
        this->pipeline_size=pipeline_size;
        this->stage_lat=stage_lat;
    }

    bool isFull();

    //push to RS
    void push(RSEntry temp);//an O(n) search.

    //method to iterate linearly and find the ready entry: optional result? returns index of ready entry. -1 if none yet. Also puts the ready rnetry in pipleine.
    int get_valid_entry();

    // update_rs(), returns index.
    int step_rs_get_final();
    //every cycle of execute, check if there is an instruction whose cycle count is pipeline length
    //if so, fetch it and do the math. The exe unit must then broadcast the result. 
    //Also delete it
    void invalidate_entry(int idx);

    //return src1, src2 and ROB entry of a RS entry at idx.
    int get_src1_at(int idx){ 
        return RS_stage_vector[idx].second.first.src1_value;
    }
    int get_src2_at(int idx){
        return RS_stage_vector[idx].second.first.src2_value;
    }
    int get_ROB_at(int idx){
        return RS_stage_vector[idx].second.first.ROB_Entry;
    }
    OpCode get_op_at(int idx){
        return RS_stage_vector[idx].second.first.op;
    }
    //capture function too. But is part of execution unit?
    void capture(int tag, int value);
    
    private:
    //or store pair: RS entry, pipline stage in a maxheap: push, pop etc sort? no not just the topmost, but otehr pipelie entryes need updation too. 
    std::vector <std::pair<bool, std::pair<RSEntry, int>>> RS_stage_vector; // bool for validity, RSEntry for the entry, int for pipeline stage.
    int size;
    int pipeline_size;
    int counter;
    int stage_lat;
};

//ex unit flow: every cycle: 
//exe()
// fun update rs: non negative entries incremented by one i = ith cycle completed. So i=pipeline_stages pe get the index, do the calc: anothe unit? and then erase the  entry from RS after rsult gotten. Current function will just provide entry index.
// update_rs(), returns index.
// do_calc.
// delete entry

// push to RS