#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Basics.h"

class ExecutionUnit {
public:
    // per-unit reservation station
    UnitType name;
    int latency;
    
    bool has_result = false; // result flag
    bool has_exception = false; // exception flag
    
    void capture(int tag, int val);
    void executeCycle();
};