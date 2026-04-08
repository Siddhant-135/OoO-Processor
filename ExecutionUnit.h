#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Basics.h"
#include "ReservationStation.h"

class ExecutionUnit {
public:
    // per-unit reservation station
    UnitType name;
    int latency;
    
    bool has_result = false; // result flag
    bool has_exception = false; // exception flag
    
    //constructor
    ExecutionUnit(UnitType name, int latency, int RS_size);
    //methods
    void capture(int tag, int val);  
    //run every cycle.
    std::pair<int,int> executeCycle();
    //add operation function
    int add(int src1, int src2){
        return src1+src2;
    }
    //mul
    int mul(int src1, int src2){
        return src1*src2;
    } 
    //div: truncated division.
    int div(int src1, int src2){
        return src1/src2;
    } 

    bool isRSFull(){
        return myRS.isFull();
    }

    void pushToRS(RSEntry temp){
        myRS.push(temp);
    }
    
    void loadToPipeline(){
        if(myRS.get_valid_entry()==-1){
            std::cout<<"No RS entry ready yet!";
        } else std::cout<<"Added new entry to execution pipeline";
    }
    private:
    RS myRS;
};