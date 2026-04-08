#pragma once
#include "Basics.h"
#include <vector>

class ROB {
    private: 
    std::vector <ROBEntry> ROB_Vector;
    int size;
    int oldest_entry_idx, youngest_entry_idx; // setting circular queue. I want them to move rightwards so to speak on adding new stuff. so youngest after push becomes former_youngest + 1 / N

    public:

    ROBEntry to_be_commited_entry(){return ROB_Vector[oldest_entry_idx];}
    ROBEntry newest_entry(){return ROB_Vector[youngest_entry_idx];}
    int newest_entry_idx(){return youngest_entry_idx;}
    void capture_results(std::vector <std::pair<int, int>> tags_values);//get the broadcasted result from the RS here. Assuming that for now only one entry from the RS gets valid
    bool is_Full() { return (((youngest_entry_idx+1)%size)==oldest_entry_idx); }
    void push(Instruction inst);
    void pop();

    ROB(int size){//constructor
        ROB_Vector.resize(size);
        this->size =  size;
        oldest_entry_idx =0; 
        youngest_entry_idx =0; 
    }
};
