#pragma once
#include "Processor.h"

class ROB {
    public:
    //methods of ROB: 
    void push(Instruction inst);
    void pop();//pop and write to the approriate register. Should be used in the commit (W) stage tho
    void capture_results(std::vector <std::pair<int, int>> tags_values);//get the broadcasted result from the RS here. Assuming that for now only one entry from the RS gets valid
    bool is_Full(){
        return (((youngest_entry+1)%size)==oldest_entry) ;
    }

    ROB(int size){//constructor
        ROB_Vector.resize(size);
        this->size =  size;
    }

    private: 
    std::vector <ROBEntry> ROB_Vector;
    //it has 2 iterators since circular queue
    int oldest_entry =0; //reset to oldest_entry+1 mod size when pop done.
    int youngest_entry =0; //at youngest_entry+1 mod size after a push is done
    int size;

};