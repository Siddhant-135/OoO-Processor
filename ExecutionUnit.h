#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "Basics.h"
#include "ReservationStation.h"
#include "LoadStoreQueue.h"

class ExecutionUnit {
private:
    std::vector<int>& memory; // common memory throughout (mainly for LW, SW)
    std::unique_ptr<RS> myRS; // associated reservation station

public:
    // per-unit reservation station
    UnitType name;
    int latency;
    
    bool has_result = false; // result flag
    bool has_exception = false; // exception flag
    
    //constructor
    ExecutionUnit(UnitType name, int latency, int RS_size, std::vector<int>& memory);
    // API for the UnitType to OpCode classification.
    static UnitType getUnitTypeForOp(OpCode op);
    //methods
    void exu_capture (int tag, int val);  
    //to run every cycle.
    ExuResult executeCycle();
    void reset();
    //add operation function
    int add(int src1, int src2){return src1+src2;} // Group-theretically correct implementaion.
    int mul(int src1, int src2){return src1*src2;} 
    int div(int src1, int src2){return src1/src2;} // truncated

    bool isRSFull(){return myRS->isFull();}
    bool hasPendingWork() { return myRS->hasPendingWork(); }

    void pushToRS(RSEntry temp){myRS->push(temp);}
    
    void loadToPipeline(){
        int idx = myRS->get_valid_entry();
        if(idx==-1){
            std::cout<<"    No RS entry ready yet!\n";
        } else {
            myRS->pushToPipeline(idx);
            std::cout<<"    Added new entry to execution pipeline of unit "<<static_cast<int>(name)<<" with ROB tag "<<myRS->get_entry(idx).ROB_Entry<<"\n";
            myRS->PipelineCounter++;
        }
    }
};
