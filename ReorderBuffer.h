#pragma once
#include "Processor.h"

class ROB {
    public:
    //methods of ROB: 
    void push(Instruction inst);
    void pop();//pop and write to the approriate register. Should be used in the commit (W) stage tho
    void dispatch();//push a command to RS, 
    void capture//get the broadcasted result from the RS here.
    //it has 2 iterators since circular queue

    ROB(int size){
        ROB_Vector.resize(size);
        this->size =  size;
    }

    private: 
    std::vector <ROBEntry> ROB_Vector;
    int oldest_entry =0; //reset to oldest_entry+1 mod size when pop done.
    int youngest_entry =0; //at youngest_entry-1 mod size whenever a push is done
    int size;

};