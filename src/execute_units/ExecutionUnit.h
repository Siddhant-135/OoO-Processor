#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "../Basics.h"
#include "ReservationStation.h"
#include "../decode_units/LoadStoreQueue.h"

class ExecutionUnit {
private:
    std::vector<int>& memory; // common memory throughout (mainly for LW, SW)
    std::unique_ptr<RS> myRS; // associated reservation station
    static constexpr long long MAX_N = 2147483647LL;
    static constexpr long long MIN_N = -2147483648LL;
    bool check_output_bound(long long value);

public:
    // per-unit reservation station
    UnitType name;
    int latency;
    
    bool has_result = false; // result flag
    bool has_exception = false; // exception flag
public:
    bool verbose = false;
    
    //constructor
    ExecutionUnit(UnitType name, int latency, int RS_size, std::vector<int>& memory, bool verbose = false);
    // API for the UnitType to OpCode classification.
    static UnitType getUnitTypeForOp(OpCode op);
    //methods
    void exu_capture (int tag, int val);  
    //to run every cycle.
    ExuResult executeCycle();
    void reset();
    //add operation function
    long long add(int src1, int src2){return (long long)src1+(long long)src2;} // Group-theretically correct implementaion.
    long long mul(int src1, int src2){return (long long)src1*(long long)src2;} 
    long long div(int src1, int src2){return (long long)src1/(long long)src2;} // truncated

    bool isRSFull(){return myRS->isFull();}
    bool hasPendingWork() { return myRS->hasPendingWork(); }

    void pushToRS(RSEntry temp){myRS->push(temp);}
    
    void loadToPipeline(){
        int idx = myRS->get_valid_entry();
        if(idx==-1){
            if (verbose) std::cout<<"    No RS entry ready yet!\n";
        } else {
            myRS->pushToPipeline(idx);
            if (verbose) std::cout<<"    Added new entry to execution pipeline of unit "<<static_cast<int>(name)<<" with ROB tag "<<myRS->get_entry(idx).ROB_Entry<<"\n";
            myRS->PipelineCounter++;
        }
    }
    void print() const;
    void setVerbose(bool v) {
        verbose = v;
        if (myRS) myRS->verbose = v;
    }
};
