#pragma once
#include "Basics.h"
#include <vector>
#include <algorithm>
#include <iostream>

class ROB {
    private: 
    std::vector <ROBEntry> ROB_Vector;
    int capacity=0;
    int number_of_entries=0;
    int oldest_entry_idx=1;
    int youngest_entry_idx=0; // setting circular queue. I want them to move rightwards so to speak on adding new stuff. so youngest after push becomes former_youngest + 1 / N

    public:

    ROBEntry to_be_commited_entry(){return ROB_Vector[oldest_entry_idx];}
    ROBEntry newest_entry(){return ROB_Vector[youngest_entry_idx];}
    int newest_entry_idx(){return youngest_entry_idx;}
    void rob_capture_results(std::vector <std::pair<int, int>> tags_values);//get the broadcasted result from the RS here. Assuming that for now only one entry from the RS gets valid
    bool is_Full() { return (number_of_entries==capacity); } // 
    bool is_Empty() { return (number_of_entries==0); } // starts with this. then youngest = oldest + 1, then +2, etc.
    // ONLY THINGS THAT EVOLVE SIZE VARIABLE ->
    void push(Instruction inst);
    bool pop(); // does it pop or not uske liye

    ROB(int size){//constructor
        ROB_Vector.resize(size);
        this->capacity = size;
        this->number_of_entries = 0;
        oldest_entry_idx =1; 
        youngest_entry_idx =0; 
    }
};
